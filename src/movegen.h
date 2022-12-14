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
void generate_partial_pawn_moves(bitboard mask, square shift, bool promotion, struct MoveList *list)
{
        while (mask) {
                square dst = lsb(mask);
                square sq  = dst - shift;

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
void generate_pawn_moves(struct Position pos, bitboard targets, bitboard _pinned, square king,
                         struct MoveList *list)
{
        bitboard pawns = extract(pos, Pawn) & pos.white;
        bitboard occ   = occupied(pos);
        bitboard enemy = occ &~ pos.white;

        bitboard en_passant = pos.white &~ occ;
        bitboard candidates = south(east(en_passant) | west(en_passant)) & pawns;

        // check that en-passant doesn't allow horizontal check (not pinned as two blockers)
        // note: this only happens when the king is on the 5th rank
        if ((king >> 3) == 4 && popcount(candidates) == 1)
        {
                bitboard rooks  = extract(pos, Rook)  &~ pos.white;
                bitboard queens = extract(pos, Queen) &~ pos.white;

                candidates |= south(en_passant);
                rooks      |= queens;

                if (rook_attacks(king, (occ | en_passant) &~ candidates) & rooks)
                        en_passant = 0;
        }

        // allow en-passant if pawn is giving check
        targets |= en_passant & north(targets);
        enemy   |= en_passant;

        bitboard pinned = pawns & _pinned;
        pawns &= ~_pinned;

        bitboard single_move = north(pawns) &~ occ;
        bitboard double_move = north(single_move & RANK3) &~ occ;

        bitboard pinned_single_move = north(pinned) & file(king) &~ occ;
        bitboard pinned_double_move = north(pinned_single_move & RANK3) &~ occ;

        single_move &= targets;
        double_move &= targets;

        pinned_single_move &= targets;
        pinned_double_move &= targets;

        // pinned orthogonal pawns cannot capture
        pinned &= ~rook_attacks(king, 0);

        bitboard east_capture = north(east(pawns)) & enemy & targets;
        bitboard west_capture = north(west(pawns)) & enemy & targets;

        bitboard pinned_east_capture = north(east(pinned)) & enemy & targets;
        bitboard pinned_west_capture = north(west(pinned)) & enemy & targets;

        // make sure pinned captures are aligned to king
        pinned_east_capture &= bishop_attacks(king, 0);
        pinned_west_capture &= bishop_attacks(king, 0);

        single_move  |= pinned_single_move;
        double_move  |= pinned_double_move;
        east_capture |= pinned_east_capture;
        west_capture |= pinned_west_capture;

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
void generate_piece_moves(piece T, struct Position pos, bitboard targets, bitboard filter,
                          bool pinned, square king, struct MoveList *list)
{
        bitboard pieces = extract(pos, T) & pos.white & filter;
        bitboard occ    = occupied(pos);

        while (pieces) {
                square sq = lsb(pieces);
                bitboard attacks = generic_attacks(T, sq, occ) & targets;

                if (pinned)
                        attacks &= line_connecting(king, sq);

                while (attacks) {
                        square dst = lsb(attacks);
                        append(list, (struct Move) { sq, dst, T, false });
                        attacks &= attacks - 1;
                }

                pieces &= pieces - 1;
        }
}


static inline
void generate_king_moves(struct Position pos, bitboard attacked, square king, struct MoveList *list)
{
        bitboard occ     = occupied(pos);
        bitboard attacks = king_attacks(king) &~ attacked &~ (pos.white & occ);

        while (attacks) {
                square sq = lsb(attacks);
                append(list, (struct Move) { king, sq, King, false });
                attacks &= attacks - 1;
        }

        // castling rights
        bitboard castle = extract(pos, Castle) & RANK1;

        enum : bitboard { QOCC = 14, QATTK = 28, KOCC = 96, KATTK = 112 };

        static struct Move queenside = { E1, C1, King, true },
                            kingside = { E1, G1, King, true };

        if (castle & (1 << A1) && !(occ & QOCC) && !(attacked & QATTK)) append(list, queenside);
        if (castle & (1 << H1) && !(occ & KOCC) && !(attacked & KATTK)) append(list,  kingside);
}


static inline
bitboard enemy_attacks(struct Position pos, bitboard *out_checkers)
{
        bitboard pawns   = extract(pos, Pawn)   &~ pos.white;
        bitboard knights = extract(pos, Knight) &~ pos.white;
        bitboard bishops = extract(pos, Bishop) &~ pos.white;
        bitboard rooks   = extract(pos, Rook)   &~ pos.white;
        bitboard queens  = extract(pos, Queen)  &~ pos.white;
        bitboard king    = extract(pos, King)   &~ pos.white;

        bishops |= queens;
        rooks   |= queens;

        bitboard our_king = extract(pos, King) & pos.white;
        bitboard occ = occupied(pos) &~ our_king;

        // ^ allow sliders to move through our king - this prevents our
        // king from walking along the checking direction

        bitboard attacked = 0, checks = 0;

        attacked |= south(east(pawns));
        attacked |= south(west(pawns));
        attacked |= king_attacks(lsb(king));

        checks |= pawns & north(east(our_king) | west(our_king));
        checks |= knights & knight_attacks(lsb(our_king));

        while (knights) {
                attacked |= knight_attacks(lsb(knights));
                knights  &= knights - 1;
        }

        while (bishops) {
                bitboard attacks = bishop_attacks(lsb(bishops), occ);
                checks   |= attacks & our_king ? bishops &- bishops : 0;
                attacked |= attacks;
                bishops  &= bishops - 1;
        }

        while (rooks) {
                bitboard attacks = rook_attacks(lsb(rooks), occ);
                checks   |= attacks & our_king ? rooks &- rooks : 0;
                attacked |= attacks;
                rooks    &= rooks - 1;
        }

        *out_checkers = checks;
        return attacked;
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
                bitboard line = line_between(king, lsb(candidates)) & occ;

                if (popcount(line) == 1)
                        pinned |= line;

                candidates &= candidates - 1;
        }

        return pinned;
}


static inline
struct MoveList generate_moves(struct Position pos)
{
        struct MoveList list = {.count = 0};

        square king = lsb(extract(pos, King) & pos.white);

        bitboard checkers;
        bitboard attacked = enemy_attacks(pos, &checkers);
        bitboard pinned   = generate_pinned(pos, king);
        bitboard targets  = ~(occupied(pos) & pos.white);

        // if in check from more than one piece, can only move king,
        // otherwise we must block the check, or capture the checking piece
        if (checkers)
                targets &= (popcount(checkers) == 1)
                        ? checkers | line_between(king, lsb(checkers)) : 0;

        // pinned knights can never move
        generate_piece_moves(Bishop, pos, targets, pinned, true, king, &list);
        generate_piece_moves(Rook,   pos, targets, pinned, true, king, &list);
        generate_piece_moves(Queen,  pos, targets, pinned, true, king, &list);

        generate_pawn_moves(pos, targets, pinned, king, &list);
        generate_piece_moves(Knight, pos, targets, ~pinned, false, king, &list);
        generate_piece_moves(Bishop, pos, targets, ~pinned, false, king, &list);
        generate_piece_moves(Rook,   pos, targets, ~pinned, false, king, &list);
        generate_piece_moves(Queen,  pos, targets, ~pinned, false, king, &list);
        generate_king_moves(pos, attacked, king, &list);

        return list;
}


static inline
struct Position make_move(struct Position pos, struct Move move)
{
        bitboard clear  = 1ULL << move.start;
                 clear |= 1ULL << move.end;

        bitboard occ = occupied(pos);
        bitboard en_passant = pos.white &~ occ;

        if (move.piece == Pawn)
                clear |= south(en_passant & clear);

        if (move.castling)
                clear |= (move.end < move.start) ? (1 << A1) : (1 << H1);

        pos.x     &= ~clear;
        pos.y     &= ~clear;
        pos.z     &= ~clear;
        pos.white &= ~clear;

        set_square(&pos, move.end, move.piece);
        pos.white |= 1ULL << move.end;

        if (move.castling) {
                square mid = (move.end + move.start) >> 1;
                set_square(&pos, mid, Rook);
                pos.white |= 1ULL << mid;
        }

        if (move.piece == King)
                pos.x ^= extract(pos, Castle) & RANK1; // remove castling rights

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
