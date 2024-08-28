#include "game_bitboard.h"
#include "memo.h"
#include "search.h"

#if THREADS
#include <pthread.h>
#endif

void* search_child(void* args) {
    SearchArgs* search_args = static_cast<SearchArgs*>(args);

    if (search_args->maxDepth == 0) {
        *(search_args->count) += 1;
        return nullptr;
    }

    BitBoard inactive = search_args->redTurn ? search_args->game.yellow : search_args->game.red;

    bool winRes = checkWin(inactive);
    if (winRes) {
        *(search_args->count) += 1;
        return nullptr;
    }

#if THREADS
    pthread_t threads[COLS];
    bool started[COLS];
    uint64_t sub_counts[COLS];
    SearchArgs child_args[COLS];
#endif

    for(uint8_t i = 0; i < COLS; i++) {
#if THREADS
        sub_counts[i] = 0;
        started[i] = false;
#endif
        BitBoard new_mask = makeMove(i, search_args->game.mask);
        if (new_mask != search_args->game.mask) {
            BitBoard new_active = new_mask ^ inactive;
            BitBoard new_red = search_args->redTurn ? new_active : inactive;
            BitBoard new_yel = search_args->redTurn ? inactive : new_active;

            if (search_args->memo->insert(new_red, new_yel, new_mask)) {
#if THREADS
                child_args[i] = SearchArgs{
                    Game{new_red, new_yel, new_mask},
                    !search_args->redTurn, search_args->maxDepth-1, &sub_counts[i], search_args->memo, search_args->spawnTilDepth
                };
                if (search_args->spawnTilDepth <= search_args->maxDepth) {
                    started[i] = true;
                    pthread_create(&threads[i], nullptr, search_child, &child_args[i]);
                } else {
                    search_child(&child_args[i]);
                }
#else
                SearchArgs child_arg = {
                    Game{new_red, new_yel, new_mask},
                    !search_args->redTurn, search_args->maxDepth-1, search_args->count, search_args->memo, search_args->spawnTilDepth
                };
                search_child(&child_arg);
#endif
            }
        }
    }

#if THREADS
    uint64_t count = 1;
    for (int i = 0; i < COLS; i++) {
        if (started[i]) {
            pthread_join(threads[i], nullptr);
        }
        count += sub_counts[i];
    }
    *(search_args->count) = count;
#endif

    return nullptr;
}

uint64_t search(
    Game game,
    bool redTurn,
    int maxDepth,
    uint64_t &count,
    Memo &memo
) {
    if (maxDepth == 0) {
        count += 1;
        return 0;
    }

    BitBoard inactive = redTurn ? game.yellow : game.red;

    bool winRes = checkWin(inactive);
    if (winRes) {
        count += 1;
        return 0;
    }

    count += 1;
    for(uint8_t i = 0; i < COLS; i++) {
        BitBoard new_mask = makeMove(i, game.mask);
        if (new_mask != game.mask) {
            BitBoard new_active = new_mask ^ inactive;
            BitBoard new_red = redTurn ? new_active : inactive;
            BitBoard new_yel = redTurn ? inactive : new_active;

            if (memo.insert(new_red, new_yel, new_mask)) {
                search(Game{new_red, new_yel, new_mask}, !redTurn, maxDepth-1, count, memo);
            }
        }
    }

    return 0;
}
