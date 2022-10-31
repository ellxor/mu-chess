#pragma once
#include "bits.h"

enum { MAX_MOVE_COUNT = 80 };

struct MoveList {
        struct Move moves[MAX_MOVE_COUNT];
        unsigned count;
};

static inline
void append(struct MoveList *list, struct Move move) {
        list->moves[list->count++] = move;
}


static inline
void generate_partial_pawn_moves(bitboard mask, square shift, bool promotion, bool pinned, square king, struct MoveList *list)
{
        while (mask) {
                square dst = lsb(mask);
                square sq  = dst - shift;

                if (pinned && !(line_connecting[king][sq] & mask & -mask)) {
                        mask &= mask - 1;
                        continue;
                }

                if (promotion) {
                        append(list, (struct Move) { sq, dst, Knight, false });
                        append(list, (struct Move) { sq, dst, Bishop, false });
                        append(list, (struct Move) { sq, dst, Rook  , false });
                        append(list, (struct Move) { sq, dst, Queen , false });
                } else {
                        append(list, (struct Move) { sq, dst, Pawn, false });
                }

                mask &= mask - 1;
        }
}


static inline
void generate_pawn_moves(struct Position pos, bitboard targets, bitboard filter, bool pinned, square king, struct MoveList *list)
{
        bitboard pawns = extract(pos, Pawn) & pos.white & filter;
        bitboard occ   = occupied(pos);
        bitboard enemy = occ &~ pos.white;

        bitboard en_passant = pos.white &~ occ;
        bitboard candidates = south(east(en_passant) | west(en_passant)) & pawns;

        if (popcount(candidates) == 1) {
                bitboard rooks  = extract(pos, Rook)  &~ pos.white;
                bitboard queens = extract(pos, Queen) &~ pos.white;

                candidates |= south(en_passant);
                rooks      |= queens;

                if (rook_attacks(king, (occ | en_passant) &~ candidates) & rooks)
                        en_passant = 0;
        }

        targets |= en_passant & north(targets);
        enemy   |= en_passant;

        bitboard single_move = north(pawns) &~ occ;
        bitboard double_move = north(single_move & RANK3) &~ occ;

        single_move &= targets;
        double_move &= targets;

        bitboard east_capture = north(east(pawns)) & enemy & targets;
        bitboard west_capture = north(west(pawns)) & enemy & targets;

        // promotions
        generate_partial_pawn_moves(single_move  & RANK8, N,   true, pinned, king, list);
        generate_partial_pawn_moves(east_capture & RANK8, N+E, true, pinned, king, list);
        generate_partial_pawn_moves(west_capture & RANK8, N+W, true, pinned, king, list);

        // non-promotions
        generate_partial_pawn_moves(single_move  &~ RANK8, N,   false, pinned, king, list);
        generate_partial_pawn_moves(double_move,           N+N, false, pinned, king, list);
        generate_partial_pawn_moves(east_capture &~ RANK8, N+E, false, pinned, king, list);
        generate_partial_pawn_moves(west_capture &~ RANK8, N+W, false, pinned, king, list);
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
void generate_piece_moves(piece T, struct Position pos, bitboard targets, bitboard filter, bool pinned, square king, struct MoveList *list)
{
        bitboard pieces = extract(pos, T) & pos.white & filter;
        bitboard occ    = occupied(pos);

        while (pieces) {
                square sq = lsb(pieces);
                bitboard attacks = generic_attacks(T, sq, occ) & targets;

                if (pinned)
                        attacks &= line_connecting[king][sq];

                while (attacks) {
                        square dst = lsb(attacks);
                        append(list, (struct Move) { sq, dst, T, false });
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
bitboard enemy_checks(struct Position pos, square king)
{
        bitboard occ     = occupied(pos);
        bitboard pawns   = extract(pos, Pawn)   & ~pos.white;
        bitboard knights = extract(pos, Knight) & ~pos.white;
        bitboard bishops = extract(pos, Bishop) & ~pos.white;
        bitboard rooks   = extract(pos, Rook)   & ~pos.white;
        bitboard queens  = extract(pos, Queen)  & ~pos.white;

        bishops |= queens;
        rooks   |= queens;

        pawns   &= north(east(1ULL << king) | west(1ULL << king));
        knights &= knight_attacks(king);
        bishops &= bishop_attacks(king, occ);
        rooks   &= rook_attacks(king, occ);

        return pawns | knights | bishops | rooks;
}


static inline
void generate_king_moves(struct Position pos, square king, struct MoveList *list)
{
        bitboard occ      = occupied(pos);
        bitboard attacked = enemy_attacks(pos);
        bitboard attacks  = king_attacks(king) &~ attacked &~ (pos.white & occ);

        while (attacks) {
                square sq = lsb(attacks);
                append(list, (struct Move) { king, sq, King, false });
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
bitboard generate_pinned(struct Position pos, square king)
{
        bitboard occ     = occupied(pos);
        bitboard bishops = extract(pos, Bishop) &~ pos.white;
        bitboard rooks   = extract(pos, Rook)   &~ pos.white;
        bitboard queens  = extract(pos, Queen)  &~ pos.white;

        bishops |= queens;
        rooks   |= queens;

        bishops &= bishop_attacks(king, bishops);
        rooks   &= rook_attacks(king, rooks);

        bitboard pinned = 0;
        bitboard candidates = bishops | rooks;

        while (candidates) {
                bitboard line = line_between[king][lsb(candidates)] & occ;
                pinned |= msb(line) &- line;
                candidates &= candidates - 1;
        }

        return pinned;
}


static inline
struct MoveList generate_moves(struct Position pos)
{
        struct MoveList list = {.count = 0};

        square king = lsb(extract(pos, King) & pos.white);
        bitboard checkers = enemy_checks(pos, king);

        bitboard targets = ~(occupied(pos) & pos.white);
        bitboard pinned = generate_pinned(pos, king);

        // if in check from more than one piece, can only move king
        if (checkers)
                targets &= (popcount(checkers) == 1) ? checkers | line_between[lsb(checkers)][king] : 0;

        generate_pawn_moves(pos, targets, pinned, true, king, &list);
        generate_piece_moves(Bishop, pos, targets, pinned, true, king, &list);
        generate_piece_moves(Rook,   pos, targets, pinned, true, king, &list);
        generate_piece_moves(Queen,  pos, targets, pinned, true, king, &list);

        generate_pawn_moves(pos, targets, ~pinned, false, king, &list);
        generate_piece_moves(Knight, pos, targets, ~pinned, false, king, &list);
        generate_piece_moves(Bishop, pos, targets, ~pinned, false, king, &list);
        generate_piece_moves(Rook,   pos, targets, ~pinned, false, king, &list);
        generate_piece_moves(Queen,  pos, targets, ~pinned, false, king, &list);
        generate_king_moves(pos, king, &list);

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
