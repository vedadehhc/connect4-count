#ifndef MEMO_H
#define MEMO_H

#include <unordered_set>
#include <tuple>
#include "bitboard.h"

#ifndef THREADS
#define THREADS 0
#endif

#if THREADS
#include <pthread.h>
#endif

#define NUM_TABLES COLS*COLS*COLS*COLS*COLS

using Key = std::tuple<BitBoard, BitBoard>;
struct KeyHasher {
    size_t operator()(const Key& key) const {
        size_t h1 = std::hash<uint64_t>{}(std::get<0>(key));
        size_t h2 = std::hash<uint64_t>{}(std::get<1>(key));
        return h1 ^ (h2 << 1);
    }
};

struct Memo {
    std::unordered_set<Key, KeyHasher> tables[NUM_TABLES];
#if THREADS
    pthread_mutex_t mutex[NUM_TABLES];
#endif

    Memo() {
#if THREADS
        for (int i = 0; i < NUM_TABLES; i++) {
            mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        }
#endif
    }

    bool insert(BitBoard red, BitBoard yel, BitBoard mask) {
        int count2 = __builtin_popcountll(mask & 0x000000003f00ULL);
        int count3 = __builtin_popcountll(mask & 0x0000003f0000ULL);
        int count4 = __builtin_popcountll(mask & 0x00003f000000ULL);
        int count5 = __builtin_popcountll(mask & 0x003f00000000ULL);
        int count6 = __builtin_popcountll(mask & 0x3f0000000000ULL);

        int ti = count2*COLS*COLS*COLS*COLS + count3*COLS*COLS*COLS + count4*COLS*COLS + count5*COLS + count6;

        std::unordered_set<Key, KeyHasher>* table = &tables[ti];
#if THREADS
        pthread_mutex_lock(&mutex[ti]);
#endif
        bool ret = table->insert(std::make_tuple(red, yel)).second;
#if THREADS
        pthread_mutex_unlock(&mutex[ti]);
#endif
        return ret;
    }
};

#endif // MEMO_H
