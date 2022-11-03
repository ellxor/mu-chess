#include "bits.h"
#include "movegen.h"
#include "perft.h"
#include "uci.h"

#include <stdio.h>
#include <time.h>


static inline
unsigned perft(struct Position pos, unsigned depth)
{
        if (depth == 1) return count_moves(pos);
        struct MoveList list = generate_moves(pos);
        if (depth == 1) return list.count;

        unsigned total = 0;

        for (unsigned i = 0; i < list.count; i++) {
                struct Position child = make_move(pos, list.moves[i]);
                total += perft(child, depth - 1);
        }

        return total;
}


struct UnitTest {
        const char *name, *fen;
        unsigned depth;
};

static const struct UnitTest tests[] =
{
        { .name = "startpos",   .fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -",                .depth = 6 },
        { .name = "kiwipete",   .fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",    .depth = 5 },
        { .name = "position 3", .fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",                               .depth = 7 },
        { .name = "position 4", .fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -",        .depth = 6 },
        { .name = "rotated 4",  .fen = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ -",        .depth = 6 },
        { .name = "position 5", .fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -",               .depth = 5 },
        { .name = "position 6", .fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", .depth = 5 },
        { .name = "promotions", .fen = "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - -",                                 .depth = 6 },
};


int main()
{
        bitbase_init();

        float avg = 0.0f;
        size_t count = sizeof tests / sizeof tests[0];

        for (unsigned i = 0; i < count; i++) {
                struct Position startpos = parse_fen(tests[i].fen);
                unsigned depth = tests[i].depth;

                clock_t t1 = clock();
                unsigned nodes = perft(startpos, depth);
                clock_t t2 = clock();

                float seconds = (float)(t2 - t1) / CLOCKS_PER_SEC;
                float mnps = nodes / seconds / 1e6f;
                avg += mnps;

                printf("%s\t%9u\t(%4.0f Mnps)\n", tests[i].name, nodes, mnps);
        }

        printf("\nAverage: %.0f Mnps\n", avg / count);
}
