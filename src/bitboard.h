#pragma once

#include <cstdint>
#include <iostream>

using BitBoard = uint64_t;

#define ROWS 6
#define COLS 7

inline BitBoard startingBoard();
inline void printBoard(BitBoard board);
inline uint64_t emptySpaceInCol(int col, BitBoard board);
inline BitBoard makeMove(uint8_t col, BitBoard board);
inline bool checkWin(BitBoard board);
inline BitBoard flip(BitBoard board);

// Implementation of the functions
inline BitBoard startingBoard() {
    return 0ULL;
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

inline uint64_t emptySpaceInCol(int col, BitBoard board) {
    uint64_t only_col = (board >> (col * 8)) & 0x0FFULL;
    return only_col + 1;
}

inline BitBoard makeMove(uint8_t col, BitBoard board) {
    uint8_t col8 = col * 8;
    uint64_t colshft = 0x1ULL << col8;
    uint64_t colmask = 0xC0ULL << col8;
    return ((board + colshft) | board) & ~colmask;
}

inline bool checkWin(BitBoard board) {
    // Vertical
    for (int c = 0; c < COLS; c++) {
        uint64_t mask = 0x0FULL << (c * 8);
        for (int offset = 0; offset <= ROWS - 4; offset++) {
            if (__builtin_popcountll(board & (mask << offset)) == 4) {
                return true;
            }
        }
    }

    // Horizontal
    for (int r = 0; r < ROWS; r++) {
        uint64_t mask = 0x01010101ULL << r;
        for (int offset = 0; offset <= COLS - 4; offset++) {
            if (__builtin_popcountll(board & (mask << (8 * offset))) == 4) {
                return true;
            }
        }
    }

    // Diagonal
    uint64_t diag_mask = (0x01) | (0x02 << 8) | (0x04 << 16) | (0x08 << 24);
    uint64_t diag_mask2 = (0x08) | (0x04 << 8) | (0x02 << 16) | (0x01 << 24);
    for (int c = 0; c <= COLS - 4; c++) {
        for (int r = 0; r <= ROWS - 4; r++) {
            if (__builtin_popcountll(board & (diag_mask << (8 * c + r))) == 4 ||
                __builtin_popcountll(board & (diag_mask2 << (8 * c + r))) == 4) {
                return true;
            }
        }
    }

    return false;
}

inline BitBoard flip(BitBoard board) {
    return __builtin_bswap64(board) >> (8 - COLS);
}
