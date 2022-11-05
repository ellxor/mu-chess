#pragma once
#include "bits.h"
#include "movegen.h"


static inline
unsigned count_pawn_moves(struct Position pos, bitboard targets, bitboard _pinned, square king)
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

        // pinned pawns on the king rank can never move
        bitboard pinned = pawns & _pinned & ~rank(king);
        pawns &= ~_pinned;

        bitboard single_move = north(pawns) &~ occ;
        bitboard double_move = north(single_move & RANK3) &~ occ;

        bitboard pinned_single_move = north(pinned) &~ occ;
        bitboard pinned_double_move = north(pinned_single_move & RANK3) &~ occ;

        single_move &= targets;
        double_move &= targets;

        pinned_single_move &= targets;
        pinned_double_move &= targets;

        bitboard east_capture = north(east(pawns)) & enemy & targets;
        bitboard west_capture = north(west(pawns)) & enemy & targets;

        bitboard pinned_east_capture = north(east(pinned)) & enemy & targets;
        bitboard pinned_west_capture = north(west(pinned)) & enemy & targets;

        // make sure pinned pawn moves are aligned with king
        pinned_east_capture &= bishop_attacks(king, 0);
        pinned_west_capture &= bishop_attacks(king, 0);
        pinned_single_move  &= file(king);
        pinned_double_move  &= file(king);

        single_move  |= pinned_single_move;
        double_move  |= pinned_double_move;
        east_capture |= pinned_east_capture;
        west_capture |= pinned_west_capture;

        unsigned count = 0;

        // promotions
        count += 4 * popcount(single_move  & RANK8);
        count += 4 * popcount(east_capture & RANK8);
        count += 4 * popcount(west_capture & RANK8);

        // non-promotions
        count += popcount(single_move  &~ RANK8);
        count += popcount(double_move          );
        count += popcount(east_capture &~ RANK8);
        count += popcount(west_capture &~ RANK8);

        return count;
}


static inline
unsigned count_piece_moves(piece T, struct Position pos, bitboard targets, bitboard filter,
                          bool pinned, square king)
{
        bitboard pieces = extract(pos, T) & pos.white & filter;
        bitboard occ    = occupied(pos);

        unsigned count = 0;

        while (pieces) {
                square sq = lsb(pieces);
                bitboard attacks = generic_attacks(T, sq, occ) & targets;

                if (pinned)
                        attacks &= line_connecting[king][sq];

                count += popcount(attacks);
                pieces &= pieces - 1;
        }

        return count;
}


static inline
unsigned count_king_moves(struct Position pos, bitboard attacked, square king)
{
        bitboard occ     = occupied(pos);
        bitboard attacks = king_attacks(king) &~ attacked &~ (pos.white & occ);

        unsigned count = popcount(attacks);

        // castling rights
        bitboard castle = extract(pos, Castle) & RANK1;

        enum : bitboard { QOCC = 14, QATTK = 28, KOCC = 96, KATTK = 112 };

        count += (castle & (1 << A1) && !(occ & QOCC) && !(attacked & QATTK));
        count += (castle & (1 << H1) && !(occ & KOCC) && !(attacked & KATTK));

        return count;
}


static inline
unsigned count_moves(struct Position pos)
{
        square king = lsb(extract(pos, King) & pos.white);

        bitboard checkers;
        bitboard attacked = enemy_attacks(pos, &checkers);
        bitboard targets  = ~(pos.white & occupied(pos));
        bitboard pinned   = generate_pinned(pos, king);

        // if in check from more than one piece, can only move king,
        // otherwise we must block the check, or capture the checking piece
        if (checkers)
                targets &= (popcount(checkers) == 1)
                        ?  checkers | line_between[king][lsb(checkers)]
                        : 0;

        unsigned count = 0;

        // pinned knights can never move
        count += count_piece_moves(Bishop, pos, targets, pinned, true, king);
        count += count_piece_moves(Rook,   pos, targets, pinned, true, king);
        count += count_piece_moves(Queen,  pos, targets, pinned, true, king);

        count += count_pawn_moves(pos, targets, pinned, king);
        count += count_piece_moves(Knight, pos, targets, ~pinned, false, king);
        count += count_piece_moves(Bishop, pos, targets, ~pinned, false, king);
        count += count_piece_moves(Rook,   pos, targets, ~pinned, false, king);
        count += count_piece_moves(Queen,  pos, targets, ~pinned, false, king);
        count += count_king_moves(pos, attacked, king);

        return count;
}
