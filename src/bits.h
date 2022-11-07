#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <x86intrin.h>

typedef uint8_t square;
typedef uint64_t bitboard;

typedef enum : uint8_t { None, Pawn, Knight, Bishop, Rook, Castle, Queen, King } piece;

struct Position { bitboard x, y, z, white; };
struct Move { square start, end; piece piece; bool castling; };

enum { N = 8, E = 1, S = -N, W = -E };
enum { A1 = 0, C1 = 2, E1 = 4, G1 = 6, H1 = 7 };

static const bitboard AFILE = 0x0101010101010101;
static const bitboard HFILE = 0x8080808080808080;
static const bitboard RANK1 = 0x00000000000000ff;
static const bitboard RANK3 = 0x0000000000ff0000;
static const bitboard RANK8 = 0xff00000000000000;

static inline bitboard north(bitboard bb) { return bb << 8; }
static inline bitboard south(bitboard bb) { return bb >> 8; }
static inline bitboard  east(bitboard bb) { return (bb &~ HFILE) << 1; }
static inline bitboard  west(bitboard bb) { return (bb &~ AFILE) >> 1; }

static inline bitboard file(square sq) { return AFILE << (sq &  7); }
static inline bitboard rank(square sq) { return RANK1 << (sq & 56); }

static inline bitboard    bswap(bitboard bb) { return    __builtin_bswap64(bb); }
static inline square        lsb(bitboard bb) { return           _tzcnt_u64(bb); }
static inline unsigned popcount(bitboard bb) { return __builtin_popcountll(bb); }
static inline bitboard     pext(bitboard a, bitboard b) { return _pext_u64(a, b); }

static inline bitboard occupied(struct Position pos) {
        return pos.x | pos.y | pos.z;
}

static inline
bitboard extract(struct Position pos, piece T)
{
        if (T == Rook) return pos.z &~ pos.y;

        bitboard bb  = (T & 1) ? pos.x : ~pos.x;
                 bb &= (T & 2) ? pos.y : ~pos.y;
                 bb &= (T & 4) ? pos.z : ~pos.z;

        return bb;
}

static inline
void set_square(struct Position *pos, square sq, piece T)
{
        pos->x |= (bitboard)((T >> 0) & 1) << sq;
        pos->y |= (bitboard)((T >> 1) & 1) << sq;
        pos->z |= (bitboard)((T >> 2) & 1) << sq;
}


#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

INCBIN(bitboard, bitbase, "data/bitbase.bin");
static struct { bitboard mask; const bitboard *attacks; } bishop[64], rook[64];

static inline bitboard knight_attacks(square sq) {
        return bitbase_data[sq];
}

static inline bitboard bishop_attacks(square sq, bitboard occ) {
        return bishop[sq].attacks[pext(occ, bishop[sq].mask)];
}

static inline bitboard rook_attacks(square sq, bitboard occ) {
        return rook[sq].attacks[pext(occ, rook[sq].mask)];
}

static inline bitboard king_attacks(square sq) {
        return bitbase_data[sq + 64];
}

static inline bitboard line_between(square a, square b) {
        return bitbase_data[107904 + 64*a + b];
}

static inline bitboard line_connecting(square a, square b) {
        return bitbase_data[112000 + 64*a + b];
}

static inline
void bitbase_init(void)
{
        size_t index = 128;

        for (square sq = 0; sq < 64; sq++) {
                bishop[sq].mask = bitbase_data[index++];
                bishop[sq].attacks = bitbase_data + index;
                index += 1 << popcount(bishop[sq].mask);

                rook[sq].mask = bitbase_data[index++];
                rook[sq].attacks = bitbase_data + index;
                index += 1 << popcount(rook[sq].mask);
        }
}
