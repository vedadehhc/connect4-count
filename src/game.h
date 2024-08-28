#ifndef GAME_H
#define GAME_H

#include <cstdint>

using BitBoard = uint64_t;

struct Game {
    BitBoard red;
    BitBoard yellow;
    BitBoard mask;
};

#endif // GAME_H
