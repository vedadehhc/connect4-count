#ifndef EVAL_SEARCH_H
#define EVAL_SEARCH_H

#include <cstdint>
#include "game_bitboard.h"

struct EvalSearchArgs {
    Game game;
    bool redTurn;
    int maxDepth;
    // int spawnTilDepth; // unused for now
};

struct EvalSearchRet {
  uint8_t move;
  int score;
};

EvalSearchRet evalSearch(EvalSearchArgs args);

#endif // EVAL_SEARCH_H