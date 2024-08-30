#include <cstdint>
#include "game_bitboard.h"
#include "eval_search.h"

const int WIN_SCORE = 10000;
const int THREE_ROW_SCORE = 200;

// struct EvalRes {
//   int score;
//   uint8_t forced_move;
// };

inline uint8_t colFromMask(uint64_t mask) {
  // mask must have exactly 1 set bit
  uint8_t col = 0;
  while (mask && (mask & 0xFF == 0)) {
    mask >>= 8;
    col++;
  }
  return col;
}

int_fast16_t evaluateWinsAndThrees(BitBoard player, BitBoard opp) {
  // player to move
  // unoptimized
  // uint64_t spot = 0ULL;
  // EvalRes res = {
  //   0, -1
  // };
  int score = 0;

  // Vertical
  for (uint8_t c = 0; c < COLS; c++) {
    uint64_t mask_pattern = 0x0FULL << (c * 8);
    for (uint8_t offset = 0; offset <= ROWS - 4; offset++) {
      uint64_t mask =  (mask_pattern << offset);
      uint8_t count = __builtin_popcountll(player & mask);
      uint8_t opp_count = __builtin_popcountll(opp & mask);
      if (count == 4) {
        // res.score = WIN_SCORE;
        // return res;
        return WIN_SCORE;
      }
      if (opp_count == 4) {
        // res.score = -WIN_SCORE;
        // return res;
        return -WIN_SCORE;
      }

      if (count == 3 && opp_count == 0) {
        score += THREE_ROW_SCORE;
      }

      if (opp_count == 3 && count == 0) {
        // uint64_t new_spot = mask & ~opp;
        // if (spot && new_spot != spot) {
        //   // opponent has 2 win spots
        //   res.score = -WIN_SCORE;
        //   return res;
        // }
        // spot = new_spot;
        // opponent has an open three
        score -= THREE_ROW_SCORE;
        // res.forced_move = colFromMask(new_spot);
      }
    }
  }

  // Horizontal
  for (uint8_t r = 0; r < ROWS; r++) {
    uint64_t mask_pattern = 0x01010101ULL << r;
    for (uint8_t offset = 0; offset <= COLS - 4; offset++) {
      uint64_t mask = mask_pattern << (8 * offset);
      uint8_t count = __builtin_popcountll(player & mask);
      uint8_t opp_count = __builtin_popcountll(opp & mask);
      if (count == 4) {
        // res.score = WIN_SCORE;
        // return res;
        return WIN_SCORE;
      }
      if (opp_count == 4) {
        // res.score = -WIN_SCORE;
        // return res;
        return -WIN_SCORE;
      }

      if (count == 3 && opp_count == 0) {
        score += THREE_ROW_SCORE;
      }

      if (opp_count == 3 && count == 0) {
        // uint64_t new_spot = mask & ~opp;
        // if (spot && new_spot != spot) {
        //   // opponent has 2 win spots
        //   res.score = -WIN_SCORE;
        //   return res;
        // }
        // spot = new_spot;
        // opponent has an open three
        score -= THREE_ROW_SCORE;
        // res.forced_move = colFromMask(new_spot);
      }
    }
  }

  // Diagonal
  uint64_t diag_mask = (0x01) | (0x02 << 8) | (0x04 << 16) | (0x08 << 24);
  uint64_t diag_mask2 = (0x08) | (0x04 << 8) | (0x02 << 16) | (0x01 << 24);
  for (uint8_t c = 0; c <= COLS - 4; c++) {
    for (uint8_t r = 0; r <= ROWS - 4; r++) {
      uint64_t mask = diag_mask << (8 * c + r);
      uint8_t count = __builtin_popcountll(player & mask);
      uint8_t opp_count = __builtin_popcountll(opp & mask);
      if (count == 4) {
        // res.score = WIN_SCORE;
        // return res;
        return WIN_SCORE;
      }
      if (opp_count == 4) {
        // res.score = -WIN_SCORE;
        // return res;
        return -WIN_SCORE;
      }

      if (count == 3 && opp_count == 0) {
        score += THREE_ROW_SCORE;
      }

      if (opp_count == 3 && count == 0) {
        // uint64_t new_spot = mask & ~opp;
        // if (spot && new_spot != spot) {
        //   // opponent has 2 win spots
        //   res.score = -WIN_SCORE;
        //   return res;
        // }
        // spot = new_spot;
        // opponent has an open three
        score -= THREE_ROW_SCORE;
        // res.forced_move = colFromMask(new_spot);
      }

      mask = diag_mask2 << (8 * c + r);
      count = __builtin_popcountll(player & mask);
      opp_count = __builtin_popcountll(opp & mask);
      if (count == 4) {
        // res.score = WIN_SCORE;
        // return res;
        return WIN_SCORE;
      }
      if (opp_count == 4) {
        // res.score = -WIN_SCORE;
        // return res;
        return -WIN_SCORE;
      }

      if (count == 3 && opp_count == 0) {
        score += THREE_ROW_SCORE;
      }

      if (opp_count == 3 && count == 0) {
        // uint64_t new_spot = mask & ~opp;
        // if (spot && new_spot != spot) {
        //   // opponent has 2 win spots
        //   res.score = -WIN_SCORE;
        //   return res;
        // }
        // spot = new_spot;
        // opponent has an open three
        score -= THREE_ROW_SCORE;
        // res.forced_move = colFromMask(new_spot);
      }
    }
  }

  return score;
}

EvalSearchRet evalSearch(EvalSearchArgs args) {
  BitBoard active = args.redTurn ? args.game.red : args.game.yellow;
  BitBoard inactive = args.redTurn ? args.game.yellow : args.game.red;
  int eval = evaluateWinsAndThrees(active, inactive);
  if (args.maxDepth == 0 || abs(eval) >= WIN_SCORE) {
    return EvalSearchRet{
      (uint8_t) -1,
      eval
    };
  }

  uint8_t bestMove = -1;
  int bestScore = -WIN_SCORE-1;

  for(uint8_t i = 0; i < COLS; i++) {
    BitBoard new_mask = makeMove(i, args.game.mask);
    if (new_mask == args.game.mask) {
      continue;
    }
    BitBoard new_active = new_mask ^ inactive;
    BitBoard new_red = args.redTurn ? new_active : inactive;
    BitBoard new_yel = args.redTurn ? inactive : new_active;
    EvalSearchArgs child_arg = {
        Game{new_red, new_yel, new_mask},
        !args.redTurn, args.maxDepth-1
    };
    int sub_score = -evalSearch(child_arg).score;
    if (sub_score > bestScore) {
      bestScore = sub_score;
      bestMove = i;
    }
  }

  return EvalSearchRet{
    bestMove,
    bestScore
  };
}