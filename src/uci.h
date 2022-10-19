#pragma once
#include <assert.h>
#include "bits.h"
#include "movegen.h"

static const piece lookup[0x80] =
{
        ['p'] = Pawn,
        ['n'] = Knight,
        ['b'] = Bishop,
        ['r'] = Rook,
        ['q'] = Queen,
        ['k'] = King,
};

static inline
struct Position parse_fen(const unsigned char *fen)
{
        struct Position pos = {0};
        bitboard white = 0, black = 0;

        square sq = 56, file = 0;

        while (sq != 8 || file != 8) {
                unsigned char c = *fen++;
                unsigned char lower = 0x20;

                if (file > 8) assert(false);
                if (file == 8) { assert(c == '/'), file = 0, sq += S+S; continue; }

                if ('1' <= c && c <= '8') {
                        unsigned offset = c - '0';
                        sq += offset, file += offset;
                }

                else if (c == '0' || c == '9') {
                        assert(false);
                }

                else {
                        piece T = lookup[c | lower];
                        if (T == None) assert(false);

                        if (c & lower) black |= 1ULL << sq;
                        else           white |= 1ULL << sq;

                        set_square(&pos, sq, T);
                        sq += 1, file += 1;
                }
        }

        assert(*fen++ == ' ');
        bool stm = true;

        switch (*fen++) {
                case 'w': stm = true; break;
                case 'b': stm = false; break;
                default : assert(false);
        }

        assert(*fen++ == ' ');
        enum { A8 = 56, H8 = 63 };

        if (*fen == '-') fen++;
        else while (*fen != ' ') {
                switch (*fen++) {
                        case 'Q': pos.x ^= 1ULL << A1; break;
                        case 'K': pos.x ^= 1ULL << H1; break;
                        case 'q': pos.x ^= 1ULL << A8; break;
                        case 'k': pos.x ^= 1ULL << H8; break;
                        default : assert(false);
                }
        }

        assert(*fen++ == ' ');
        bitboard en_passant = 0;

        if (*fen != '-') {
                unsigned char file = *fen++;
                unsigned char rank = *fen++;

                file -= 'a';
                rank -= '8';

                assert(file <= 8 && rank <= 8);
                square ep = 8*rank + file;
                en_passant = 1ULL << ep;
        }

        if (stm) {
                pos.white = white | en_passant;
        } else {
                pos.x = bswap(pos.x);
                pos.y = bswap(pos.y);
                pos.z = bswap(pos.z);
                pos.white = bswap(black | en_passant);
        }

        return pos;
}
