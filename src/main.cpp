#include <iostream>
#include <string>
#include <cassert>
#include <chrono>
#include "game_bitboard.h"
// #include "search.h"
#include "memo.h"
#include "eval_search.h"

int main(int argc, char **argv) {
    Game game = startGame();
    GameState state;
    while((state = isFinished(game)) == GameState::ONGOING) {
        int move;
        prettyPrintGame(game);
        EvalSearchArgs args = EvalSearchArgs{
            game,
            __builtin_popcountll(game.red) == __builtin_popcountll(game.yellow),
            10
        };
        EvalSearchRet ret = evalSearch(args);
        std::cout << "Best move: " << ret.move + 1 << ", Score: " << ret.score << "\n";
        std::cout << "Make your move (1-7): ";
        std::cin >> move;
        if (move < 1 || move > 7) {
            std::cout << "Your move must be between 1-7, try again.\n";
        } else {
            game = makeMoveGame(game, move - 1);
        }
    }
    std::cout << "\n===============\n";
    if (state == GameState::RED_WIN) {
        std::cout << "RED wins!\n";
    } else if (state == GameState::YELLOW_WIN) {
        std::cout << "YELLOW wins!\n";
    } else {
        std::cout << "Draw!\n";
    }
}
