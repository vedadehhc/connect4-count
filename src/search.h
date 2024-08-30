#ifndef SEARCH_H
#define SEARCH_H

#include <cstdint>
#include "game_bitboard.h"
#include "memo.h"

#ifndef THREADS
#define THREADS 0
#endif

#if THREADS
#include <pthread.h>
#endif

struct SearchArgs {
    Game game;
    bool redTurn;
    int maxDepth;
    uint64_t *count;
    Memo *memo;
    int spawnTilDepth;
};

void* search_child(void* args);

uint64_t search(
    Game game,
    bool redTurn,
    int maxDepth,
    uint64_t &count,
    Memo &memo
);

#endif // SEARCH_H
