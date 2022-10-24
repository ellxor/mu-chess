#pragma once
#include "bits.h"

enum { MAX_MOVE_COUNT = 128 };

struct MoveList {
        struct Move moves[MAX_MOVE_COUNT];
        unsigned count;
};

static inline
void append(struct MoveList *list, struct Move move) {
        list->moves[list->count++] = move;
}


static inline
void generate_partial_pawn_moves(bitboard mask, square shift, bool promotion, struct MoveList *list)
{
        while (mask) {
                square dst = lsb(mask);
                square sq  = dst - shift;

                if (promotion) {
                        append(list, (struct Move) { sq, dst, Knight });
                        append(list, (struct Move) { sq, dst, Bishop });
                        append(list, (struct Move) { sq, dst, Rook   });
                        append(list, (struct Move) { sq, dst, Queen  });
                } else {
                        append(list, (struct Move) { sq, dst, Pawn });
                }

                mask &= mask - 1;
        }
}


static inline
void generate_pawn_moves(struct Position pos, bitboard targets, bitboard filter, struct MoveList *list)
{
        bitboard pawns = extract(pos, Pawn) & pos.white & filter;
        bitboard occ   = occupied(pos);
        bitboard enemy = occ ^ pos.white;

        bitboard en_passant = pos.white &~ occ;
        targets |= en_passant & north(targets);

        bitboard single_move = north(pawns) &~ occ;
        bitboard double_move = north(single_move & RANK3) &~ occ;

        single_move &= targets;
        double_move &= targets;

        bitboard east_capture = north(east(pawns)) & enemy & targets;
        bitboard west_capture = north(west(pawns)) & enemy & targets;

        // promotions
        generate_partial_pawn_moves(single_move  & RANK8, N,   true, list);
        generate_partial_pawn_moves(east_capture & RANK8, N+E, true, list);
        generate_partial_pawn_moves(west_capture & RANK8, N+W, true, list);

        // non-promotions
        generate_partial_pawn_moves(single_move  &~ RANK8, N,   false, list);
        generate_partial_pawn_moves(double_move,           N+N, false, list);
        generate_partial_pawn_moves(east_capture &~ RANK8, N+E, false, list);
        generate_partial_pawn_moves(west_capture &~ RANK8, N+W, false, list);
}


static inline
bitboard generic_attacks(piece T, square sq, bitboard occ)
{
        switch (T) {
                case Knight: return knight_attacks(sq);
                case Bishop: return bishop_attacks(sq, occ);
                case Rook:   return rook_attacks(sq, occ);
                case Queen:  return bishop_attacks(sq, occ) | rook_attacks(sq, occ);
                default:     return 0;
        }
}


static inline
void generate_piece_moves(piece T, struct Position pos, bitboard targets, bitboard filter, struct MoveList *list)
{
        bitboard pieces = extract(pos, T) & pos.white & filter;
        bitboard occ    = occupied(pos);

        while (pieces) {
                square sq = lsb(pieces);
                bitboard attacks = generic_attacks(T, sq, occ) & targets;

                while (attacks) {
                        square dst = lsb(attacks);
                        append(list, (struct Move) { sq, dst, T });
                        attacks &= attacks - 1;
                }

                pieces &= pieces - 1;
        }
}


static inline
bitboard enemy_attacks(struct Position pos)
{
        bitboard pawns   = extract(pos, Pawn)   & ~pos.white;
        bitboard knights = extract(pos, Knight) & ~pos.white;
        bitboard bishops = extract(pos, Bishop) & ~pos.white;
        bitboard rooks   = extract(pos, Rook)   & ~pos.white;
        bitboard queens  = extract(pos, Queen)  & ~pos.white;
        bitboard king    = extract(pos, King)   & ~pos.white;

        bishops |= queens;
        rooks   |= queens;

        bitboard attacks = 0;
        bitboard occ = occupied(pos) &~ (extract(pos, King) & pos.white);
        // allow sliders to move through our king

        attacks |= south(east(pawns));
        attacks |= south(west(pawns));

        while (knights) {
                attacks |= knight_attacks(lsb(knights));
                knights &= knights - 1;
        }

        while (bishops) {
                attacks |= bishop_attacks(lsb(bishops), occ);
                bishops &= bishops - 1;
        }

        while (rooks) {
                attacks |= rook_attacks(lsb(rooks), occ);
                rooks   &= rooks - 1;
        }

        attacks |= king_attacks(lsb(king));
        return attacks;
}


static inline
bitboard enemy_checks(struct Position pos)
{
        bitboard king = extract(pos, King) & pos.white;
        square sq = lsb(king);

        bitboard occ = occupied(pos);

        bitboard pawns   = extract(pos, Pawn)   & ~pos.white;
        bitboard knights = extract(pos, Knight) & ~pos.white;
        bitboard bishops = extract(pos, Bishop) & ~pos.white;
        bitboard rooks   = extract(pos, Rook)   & ~pos.white;
        bitboard queens  = extract(pos, Queen)  & ~pos.white;

        bishops |= queens;
        rooks   |= queens;

        pawns   &= north(east(king) | west(king));
        knights &= knight_attacks(sq);
        bishops &= bishop_attacks(sq, occ);
        rooks   &= rook_attacks(sq, occ);

        return pawns | knights | bishops | rooks;
}


static inline
void generate_king_moves(struct Position pos, struct MoveList *list)
{
        square sq = lsb(extract(pos, King) & pos.white);

        bitboard occ      = occupied(pos);
        bitboard attacked = enemy_attacks(pos);
        bitboard attacks  = king_attacks(sq) &~ attacked &~ (pos.white & occ);

        while (attacks) {
                square dst = lsb(attacks);
                append(list, (struct Move) { sq, dst, King });
                attacks &= attacks - 1;
        }

        // castling
        bitboard castle = extract(pos, Castle) & RANK1;

        static const bitboard KATTK = 0b01110000, KOCC = 0b01100000;
        static const bitboard QATTK = 0b00011100, QOCC = 0b00001110;

        if (castle & (1 << A1) && !(occ & QOCC) && !(attacked & QATTK)) append(list, (struct Move) { E1, C1, King, true });
        if (castle & (1 << H1) && !(occ & KOCC) && !(attacked & KATTK)) append(list, (struct Move) { E1, G1, King, true });
}


static inline
bitboard generate_pinned(struct Position pos)
{
        bitboard bishops = extract(pos, Bishop) &~ pos.white;
        bitboard rooks   = extract(pos, Rook)   &~ pos.white;
        bitboard queens  = extract(pos, Queen)  &~ pos.white;

        bishops |= queens;
        rooks   |= queens;

        bitboard king = extract(pos, King) & pos.white;
        bitboard occ = occupied(pos);
        bitboard en_passant = pos.white &~ occ;

        square sq = lsb(king);

        bishops &= bishop_attacks(sq, bishops);
        rooks   &= rook_attacks(sq, rooks);

        bitboard pinned = 0;
        bitboard candidates = bishops | rooks;

        while (candidates) {
                bitboard bit = candidates & -candidates;
                bitboard line = line_between(bit, king) & occ;
                line &= ~south(en_passant);
                if (popcount(line) == 1) pinned |= line;
                candidates &= candidates - 1;
        }

        return pinned;
}


static inline
void filter_pinned_moves(struct Position pos, square king, struct MoveList *list)
{
        bitboard bishops = extract(pos, Bishop) &~ pos.white;
        bitboard rooks   = extract(pos, Rook)   &~ pos.white;
        bitboard queens  = extract(pos, Queen)  &~ pos.white;

        bitboard occ = occupied(pos);
        bitboard en_passant = pos.white &~ occ;

        bishops |= queens;
        rooks   |= queens;

        unsigned count = list->count;
        list->count = 0;

        for (unsigned i = 0; i < count; i++) {
                struct Move move = list->moves[i];

                bitboard mask = 1ULL << move.start;
                bitboard dst = 1ULL << move.end;

                if (move.piece == Pawn)
                        mask |= south(en_passant & dst);

                bitboard nocc = (occ & ~mask) | dst;
                bitboard check = (bishops &~ dst & bishop_attacks(king, nocc))
                               | (rooks &~ dst & rook_attacks(king, nocc));

                if (check) continue;
                append(list, move);
        }
}


static inline
struct MoveList generate_moves(struct Position pos)
{
        struct MoveList list = {.count = 0};

        bitboard checkers = enemy_checks(pos);
        bitboard king = extract(pos, King) & pos.white;

        bitboard targets = ~(occupied(pos) & pos.white);
        bitboard pinned = generate_pinned(pos);

        // if in check from more than one piece, can only move king
        if (checkers)
                targets &= (popcount(checkers) == 1) ? checkers | line_between(checkers, king) : 0;

        generate_pawn_moves(pos, targets, pinned, &list);
        generate_piece_moves(Knight, pos, targets, pinned, &list);
        generate_piece_moves(Bishop, pos, targets, pinned, &list);
        generate_piece_moves(Rook,   pos, targets, pinned, &list);
        generate_piece_moves(Queen,  pos, targets, pinned, &list);
        filter_pinned_moves(pos, lsb(king), &list);

        generate_pawn_moves(pos, targets, ~pinned, &list);
        generate_piece_moves(Knight, pos, targets, ~pinned, &list);
        generate_piece_moves(Bishop, pos, targets, ~pinned, &list);
        generate_piece_moves(Rook,   pos, targets, ~pinned, &list);
        generate_piece_moves(Queen,  pos, targets, ~pinned, &list);
        generate_king_moves(pos, &list);

        return list;
}


static inline
void set_square(struct Position *pos, square sq, piece T)
{
        pos->x |= (bitboard)((T >> 0) & 1) << sq;
        pos->y |= (bitboard)((T >> 1) & 1) << sq;
        pos->z |= (bitboard)((T >> 2) & 1) << sq;
}


static inline
struct Position make_move(struct Position pos, struct Move move)
{
        bitboard clear =  1ULL << move.start;
                 clear |= 1ULL << move.end;

        bitboard occ = occupied(pos);
        bitboard ep_mask = pos.white &~ occ;

        if (move.piece == Pawn)
                clear |= south(ep_mask & clear);

        if (move.castling)
                clear |= (move.end < move.start) ? (1 << A1) : (1 << H1);

        pos.x     &= ~clear;
        pos.y     &= ~clear;
        pos.z     &= ~clear;
        pos.white &= ~clear;

        pos.white |= 1ULL << move.end;
        set_square(&pos, move.end, move.piece);

        if (move.castling) {
                square mid = (move.start + move.end) >> 1;
                pos.white |= 1ULL << mid;
                set_square(&pos, mid, Rook);
        }

        // remove castling rights
        if (move.piece == King)
                pos.x ^= extract(pos, Castle) & RANK1;

        bitboard black = occupied(pos) &~ pos.white;

        // update en-passant
        if (move.piece == Pawn && move.end - move.start == N+N)
                black |= 256ULL << move.start;

        pos.x     = bswap(pos.x);
        pos.y     = bswap(pos.y);
        pos.z     = bswap(pos.z);
        pos.white = bswap(black);

        return pos;
}
