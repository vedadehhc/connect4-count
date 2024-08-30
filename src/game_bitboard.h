#ifndef GAME_BITBOARD_H
#define GAME_BITBOARD_H

#include <cstdint>
#include <iostream>

using BitBoard = uint64_t;

#define ROWS 6
#define COLS 7

struct Game {
    BitBoard red;
    BitBoard yellow;
    BitBoard mask;
};

enum GameState {
    RED_WIN,
    YELLOW_WIN,
    DRAW,
    ONGOING
};

// TODO: figure out how to make these non-inline
inline BitBoard startingBoard();
inline Game startGame();
inline void printBoard(BitBoard board);
inline uint64_t emptySpaceInCol(uint8_t col, BitBoard board);
inline BitBoard makeMove(uint8_t col, BitBoard board);
inline Game makeMoveGame(Game game, int col);
inline bool checkWin(BitBoard board);
inline GameState isFinished(Game game);
inline BitBoard flip(BitBoard board);

// Implementation of the functions
inline BitBoard startingBoard() {
    return 0ULL;
}

inline Game startGame() {
    return Game{startingBoard(), startingBoard(), startingBoard()};
}

inline void printBoard(BitBoard board) {
    for (int row = ROWS - 1; row >= 0; row--) {
        for (int col = 0; col < COLS; col++) {
            uint64_t val = (board >> (col * 8 + row)) & 0x1ULL;
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

inline void prettyPrintGame(Game game) {
    for (int col = 0; col < COLS; col++) {
        std::cout << "|" << col + 1;
    }
    std::cout << "|\n";
    // TODO: add color stuff
    for (int row = ROWS - 1; row >= 0; row--) {
        std::cout << "|";
        for (int col = 0; col < COLS; col++) {
            uint64_t is_red = (game.red >> (col * 8 + row)) & 0x1ULL;
            uint64_t is_yel = (game.yellow >> (col * 8 + row)) & 0x1ULL;
            char val = is_red ? 'R' : is_yel ? 'Y' : ' '; 
            std::cout << val << "|";
        }
        std::cout << "\n";
    }
    std::string s(2*COLS+1, '-');
    std::cout << s << "\n";
}

inline uint64_t emptySpaceInCol(uint8_t col, BitBoard board) {
    uint64_t only_col = (board >> (col * 8)) & 0x0FFULL;
    return only_col + 1;
}

inline BitBoard makeMove(uint8_t col, BitBoard board) {
    uint8_t col8 = col * 8;
    uint64_t colshft = 0x1ULL << col8;
    uint64_t colmask = 0xC0ULL << col8;
    return ((board + colshft) | board) & ~colmask;
}

inline Game makeMoveGame(Game game, int col) {
    game.mask = makeMove(col, game.mask);
    if (__builtin_popcountll(game.red) == __builtin_popcountll(game.yellow)) {
        // red turn
        game.red = game.mask ^ game.yellow;
    } else {
        game.yellow = game.mask ^ game.red;
    }
    return game;
}

inline bool checkWin(BitBoard board) {
    // Vertical
    for (uint8_t c = 0; c < COLS; c++) {
        uint64_t mask = 0x0FULL << (c * 8);
        for (uint8_t offset = 0; offset <= ROWS - 4; offset++) {
            if (__builtin_popcountll(board & (mask << offset)) == 4) {
                return true;
            }
        }
    }

    // Horizontal
    for (uint8_t r = 0; r < ROWS; r++) {
        uint64_t mask = 0x01010101ULL << r;
        for (uint8_t offset = 0; offset <= COLS - 4; offset++) {
            if (__builtin_popcountll(board & (mask << (8 * offset))) == 4) {
                return true;
            }
        }
    }

    // Diagonal
    uint64_t diag_mask = (0x01) | (0x02 << 8) | (0x04 << 16) | (0x08 << 24);
    uint64_t diag_mask2 = (0x08) | (0x04 << 8) | (0x02 << 16) | (0x01 << 24);
    for (uint8_t c = 0; c <= COLS - 4; c++) {
        for (uint8_t r = 0; r <= ROWS - 4; r++) {
            if (__builtin_popcountll(board & (diag_mask << (8 * c + r))) == 4 ||
                __builtin_popcountll(board & (diag_mask2 << (8 * c + r))) == 4) {
                return true;
            }
        }
    }

    return false;
}

inline GameState isFinished(Game game) {
    // unoptimized function, only use in interactive mode
    if (checkWin(game.red)) {
        return RED_WIN;
    }
    if (checkWin(game.yellow)) {
        return YELLOW_WIN;
    }
    if (__builtin_popcountll(game.mask) == COLS * ROWS) {
        return DRAW;
    }
    return ONGOING;
}


inline BitBoard flip(BitBoard board) {
    return __builtin_bswap64(board) >> (8 - COLS);
}

#endif // GAME_BITBOARD_H