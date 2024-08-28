#include <iostream>
#include <string>
#include <cassert>
#include <chrono>
#include "bitboard.h"
#include "game.h"
#include "search.h"
#include "memo.h"

int main(int argc, char **argv) {
    unsigned long long ANS[43] = {
        1, 7, 49, 238, 1120, 4263, 16422, 54859, 184275, 558186, 1662623, 4568683,
        12236101, 30929111, 75437595, 176541259, 394591391, 858218743, 1763883894,
        3568259802, 6746155945, 12673345045, 22010823988, 38263228189, 60830813459,
        97266114959, 140728569039, 205289508055, 268057611944, 352626845666,
        410378505447, 479206477733, 488906447183, 496636890702, 433471730336,
        370947887723, 266313901222, 183615682381, 104004465349, 55156010773,
        22695896495, 7811825938, 1459332899
    };

    int maxDepth = 3;
    if (argc == 2) {
        std::string depth_str = std::string(argv[1]);
        maxDepth = std::stoi(depth_str);
    }
    std::cout << "Searching up to depth " << maxDepth << "\n";

    auto t1 = std::chrono::high_resolution_clock::now();

    Memo memo;
    std::cout << "starting!\n";
    uint64_t count = 0;

#if THREADS
    SearchArgs args = SearchArgs {
        Game{startingBoard(), startingBoard(), startingBoard()}, true, maxDepth, &count, &memo, maxDepth-1
    };
    search_child(&args);
#else
    search(Game{startingBoard(), startingBoard(), startingBoard()}, true, maxDepth, count, memo);
#endif

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "search() took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
              << " milliseconds\n";

    std::cout << "Num positions up to depth=" << maxDepth << " is " << count << "\n";
    unsigned long long ans = 0;
    for (int i = 0; i <= maxDepth; i++) {
        ans += ANS[i];
    }
    if (ans != count) {
        std::cout << "Didn't match official answer: " << ans << "\n";
    } else {
        std::cout << "PASSED\n";
    }

    return 0;
}
