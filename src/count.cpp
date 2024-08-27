#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cassert>
#include <tuple>
#include <chrono>
#include <bit>
#include <pthread.h>


#define ull unsigned long long
#define ROWS 6
#define COLS 7

#define FLIP_OPT 0

using namespace std;
using BitBoard = uint64_t;
// from LSB to MSB, board is laid out as
// col0, col1, ...., col6
// = (c1,bottom row), (c1, r2) ... (c1, r6), 0, 0
// i.e. each column is byte-aligned

BitBoard startingBoard() {
  return 0ULL;
}

void printBoard(BitBoard board) {
  for (int row = ROWS-1; row >= 0; row--) {
    for (int col = 0; col < COLS; col++) {
      uint64_t val = (board >> (col*8 + row)) & 0x1ULL;
      cout << val << " ";
    }
    cout << "\n";
  }
  cout << "\n";
}

uint64_t inline emptySpaceInCol(int col, BitBoard board) {
  uint64_t only_col = (board >> (col * 8)) & 0x0FFULL;
  return only_col+1;
}

long long moveTime = 0;
BitBoard makeMove(uint8_t col, BitBoard board) {
  // naive implementation
  // uint64_t empty_space = emptySpaceInCol(col, board);
  // if (empty_space > (1 << (ROWS-1))) {
  //   return board;
  // }
  // board = board ^ (empty_space << (col*8));
  // return board;

  // maybe better?
  uint8_t col8 = col*8;
  uint64_t colshft = 0x1ULL << col8;
  uint64_t colmask = 0xC0ULL << col8;
  return ((board + colshft) | board) & ~colmask;
}

ull winTime = 0;

bool checkWin(BitBoard board) {
  // naive for now
  for (int c = 0; c < COLS; c++) {
    uint64_t mask = 0x0FULL << (c*8);
    for (int offset = 0; offset <= ROWS-4; offset++) {
      if (__builtin_popcountll(board & (mask << offset)) == 4) {
        return true;
      }
    }
  }

  // row
  for (int r = 0; r < ROWS; r++) {
    uint64_t mask = 0x01010101ULL << r;
    for (int offset = 0; offset <= COLS-4; offset++) {
      if (__builtin_popcountll(board & (mask << (8*offset))) == 4) {
        return true;
      }
    }
  }

  // diag / 
  uint64_t diag_mask = (0x01) | (0x02 << 8) | (0x04 << 16) | (0x08 << 24);
  // diag `\` 
  uint64_t diag_mask2 = (0x08) | (0x04 << 8) | (0x02 << 16) | (0x01 << 24);
  for (int c = 0; c <= COLS-4; c++) {
    for (int r = 0; r <= ROWS-4; r++) {
      if (__builtin_popcountll(board & (diag_mask << (8*c + r))) == 4 || __builtin_popcountll(board & (diag_mask2 << (8*c + r))) == 4) {
        return true;
      }
    }
  }

  return false;
}

BitBoard inline flip(BitBoard board) {
  return __builtin_bswap64(board) >> (8-COLS);
}


using Key = tuple<BitBoard, BitBoard>; // red board, yellow board
struct KeyHasher {
  size_t operator() (const Key& key) const {
    size_t h1 = hash<uint64_t>{}(get<0>(key));
    size_t h2 = hash<uint64_t>{}(get<1>(key));
    return h1 ^ (h2 << 1);
  }
};
using Key2 = __uint128_t;

ull insert_or_find(unordered_map<Key, ull, KeyHasher> memo, BitBoard red, BitBoard yel) {
  Key k1 = make_tuple(red, yel);
  auto it1 = memo.find(k1);
  if (it1 != memo.end()) {
    return 0ULL;
  }

  Key k2 = make_tuple(flip(red), flip(yel));
  auto it2 = memo.find(k1);
  if (it2 != memo.end()) {
    ull val = it2->second;
    memo.insert(make_pair(k1, val));
    return val;
  }

  return -1ULL;
}

#ifndef THREADS
#define THREADS 0
#endif
#define NUM_TABLES COLS*COLS*COLS*COLS*COLS

#if THREADS
  pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

struct Memo {
  unordered_set<Key, KeyHasher> tables[NUM_TABLES];
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
    // int count1 = __builtin_popcountll(mask & 0x00000000003fULL);
    int count2 = __builtin_popcountll(mask & 0x000000003f00ULL);
    int count3 = __builtin_popcountll(mask & 0x0000003f0000ULL);
    int count4 = __builtin_popcountll(mask & 0x00003f000000ULL);
    int count5 = __builtin_popcountll(mask & 0x003f00000000ULL);
    int count6 = __builtin_popcountll(mask & 0x3f0000000000ULL);

    int ti = /*count1*COLS*COLS*COLS*COLS*COLS + */ count2*COLS*COLS*COLS*COLS + count3*COLS*COLS*COLS + count4*COLS*COLS + count5*COLS + count6;

    unordered_set<Key, KeyHasher>* table = &tables[ti];
#if THREADS
    pthread_mutex_lock(&mutex[ti]);
#endif
    // Critical section
    bool ret = table->insert(make_tuple(red, yel)).second;
#if THREADS
    pthread_mutex_unlock(&mutex[ti]);
#endif
    return ret;

  }
};

void test() {
  BitBoard board = startingBoard();
  printBoard(board);
  for (int i = 0; i < 7; i++) {
    board = makeMove(0, board);
    printBoard(board);
  }
  assert(board == 0x3f);
  for (int i = 0; i < 7; i++) {
    board = makeMove(2, board);
  }
  printBoard(board);
  assert(board == 0x3f003f);

  cout << "\n\nTWO BOARDS\n";
  BitBoard red = startingBoard();
  BitBoard yel = startingBoard();
  red = makeMove(0, red);
  BitBoard mask = red | yel;
  BitBoard new_mask = makeMove(0, mask);
  yel = new_mask ^ red;
  printBoard(yel);
  assert(yel == 0x02);
  mask = red | yel;
  new_mask = makeMove(0, mask);
  red = new_mask ^ yel;
  printBoard(red);
  assert(red == 0x05);

  Key key = make_tuple(red, yel);
  unordered_set<Key, KeyHasher> memo;
  memo.insert(key);
  assert(memo.find(key) != memo.end());

  red = startingBoard();
  red = makeMove(0, red);
  red = makeMove(0, red);
  red = makeMove(0, red);
  red = makeMove(0, red);
  assert(checkWin(red));

  cout << "hor win\n";
  red = startingBoard();
  red = makeMove(1, red);
  red = makeMove(2, red);
  red = makeMove(3, red);
  red = makeMove(4, red);
  printBoard(red);
  printf("RED: 0x%llx\n", red);
  uint64_t check = (red & (0x01010101ULL << 8)) ;
  printf("CHECK: 0x%llx, %d\n", check, __builtin_popcountll(check));
  assert(checkWin(red));

  cout << "\ndiag / win\n";
  red = startingBoard();
  yel = startingBoard();
  red = makeMove(1, red);
  yel = makeMove(2, red | yel) ^ red;
  red = makeMove(2, red | yel) ^ yel;
  yel = makeMove(3, red | yel) ^ red;
  yel = makeMove(3, red | yel) ^ red;
  red = makeMove(3, red | yel) ^ yel;
  yel = makeMove(4, red | yel) ^ red;
  yel = makeMove(4, red | yel) ^ red;
  yel = makeMove(4, red | yel) ^ red;
  red = makeMove(4, red | yel) ^ yel;
  printBoard(red);
  printBoard(yel);
  assert(checkWin(red));
}

struct Game {
  BitBoard red;
  BitBoard yellow;
  BitBoard mask;
};

struct SearchArgs {
  Game game;
  bool redTurn;
  int maxDepth;
  ull *count;
  Memo *memo;
  int spawnTilDepth;
};

void* search_child(
  void* args
) {
  SearchArgs* search_args = static_cast<SearchArgs*>(args);

  if (search_args->maxDepth == 0) {
    *(search_args->count) += 1;
    return nullptr;
  }

  BitBoard inactive = search_args->redTurn ? search_args->game.yellow : search_args->game.red;

  // auto wint1 = std::chrono::high_resolution_clock::now();
  bool winRes = checkWin(inactive);
  // auto wint2 = std::chrono::high_resolution_clock::now();
  // winTime += std::chrono::duration_cast<std::chrono::milliseconds>(wint2-wint1).count();
  if (winRes) {
    // if is win, also return 1 and stop search
    *(search_args->count) += 1;
    return nullptr;
  }

  // cur position is valid
  *(search_args->count) += 1;
  pthread_t threads[COLS];
  bool started[COLS];
  ull sub_counts[COLS];
  SearchArgs child_args[COLS];

  for(uint8_t i = 0; i < COLS; i++) {
    sub_counts[i] = 0;
    started[i] = false;
    BitBoard new_mask = makeMove(i, search_args->game.mask);
    if (new_mask != search_args->game.mask) {
      BitBoard new_active = new_mask ^ inactive;
      BitBoard new_red = search_args->redTurn ? new_active : inactive;
      BitBoard new_yel = search_args->redTurn ? inactive : new_active;

      if (search_args->memo->insert(new_red, new_yel, new_mask)) {
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
      }
    }
  }

  for (int i = 0; i < COLS; i++) {
    if (started[i]) {
      pthread_join(threads[i], nullptr);
    }
    *(search_args->count) += sub_counts[i];
  }
#if THREADS
  // pthread_mutex_lock(&g_lock);
  // printBoard(search_args->game.mask);
  // cout << "COUNT = " << *(search_args->count) << "\n\n-----\n";
  // pthread_mutex_unlock(&g_lock);
#endif

  return nullptr;
}

ull hashTime = 0;
void* search(
  Game game,
  bool redTurn, 
  int maxDepth,
  ull &count,
#if FLIP_OPT
  unordered_map<Key, ull, KeyHasher> &memo,
#else
  // unordered_set<Key, KeyHasher> &memo
  Memo &memo
#endif
) {
  if (maxDepth == 0) {
    count += 1;
    return nullptr;
  }

  BitBoard inactive = redTurn ? game.yellow : game.red;

  // auto wint1 = std::chrono::high_resolution_clock::now();
  bool winRes = checkWin(inactive);
  // auto wint2 = std::chrono::high_resolution_clock::now();
  // winTime += std::chrono::duration_cast<std::chrono::milliseconds>(wint2-wint1).count();
  if (winRes) {
    // if is win, also return 1 and stop search
    count += 1;
    return nullptr;
  }

  // cur position is valid
  count += 1;
  for(uint8_t i = 0; i < COLS; i++) {
    BitBoard new_mask = makeMove(i, game.mask);
    if (new_mask != game.mask) {
      BitBoard new_active = new_mask ^ inactive;
      BitBoard new_red = redTurn ? new_active : inactive;
      BitBoard new_yel = redTurn ? inactive : new_active;

#if FLIP_OPT
      Key k1 = make_tuple(new_red, new_yel);
      auto it1 = memo.find(k1);
      if (it1 != memo.end()) {
        // already counted
        continue;
      }

      Key k2 = make_tuple(flip(new_red), flip(new_yel));
      auto it2 = memo.find(k1);
      if (it2 != memo.end()) {
        // counted the flip
        ull val = it2->second;
        memo.insert(make_pair(k1, val));
        count += val;
        continue;
      }

      ull val = search(Game{new_red, new_yel, new_mask}, !redTurn, maxDepth-1, memo);
      memo.insert(make_pair(k1, val));
      count += val;
#else
      // Key key = make_tuple(new_red, new_yel);
      // Key2 key = (__uint128_t(new_red) | __uint128_t(new_yel) << 64);
      // auto hasht1 = std::chrono::high_resolution_clock::now();
      // auto res = memo.insert(key);
      // auto hasht2 = std::chrono::high_resolution_clock::now();
      // hashTime += std::chrono::duration_cast<std::chrono::milliseconds>(hasht2-hasht1).count();
      if (memo.insert(new_red, new_yel, new_mask)) {
        search(Game{new_red, new_yel, new_mask}, !redTurn, maxDepth-1, count, memo);
      }
#endif
    }
  }

  return nullptr;
}

int main(int argc, char **argv) {
  unsigned long long ANS [43] = {
    1,
    7,
    49,
    238,
    1120,
    4263,
    16422,
    54859,
    184275,
    558186,
    1662623,
    4568683,
    12236101,
    30929111,
    75437595,
    176541259,
    394591391,
    858218743,
    1763883894,
    3568259802,
    6746155945,
    12673345045,
    22010823988,
    38263228189,
    60830813459,
    97266114959,
    140728569039,
    205289508055,
    268057611944,
    352626845666,
    410378505447,
    479206477733,
    488906447183,
    496636890702,
    433471730336,
    370947887723,
    266313901222,
    183615682381,
    104004465349,
    55156010773,
    22695896495,
    7811825938,
    1459332899
  };
  
  // test();
  // return 0;
  
  int maxDepth = 3;
  if (argc == 2) {
    string depth_str = string(argv[1]);
    maxDepth = stoi(depth_str);
  }
  cout << "Searching up to depth " << maxDepth << "\n";


  auto t1 = std::chrono::high_resolution_clock::now();
  // int cumCount = 0;
  // for (int depth = 0; depth <= maxDepth; depth++) {
  //   unordered_set<Key, KeyHasher> memo;
  //   unsigned long long count = search(startingBoard(), startingBoard(), true, depth, memo);
  //   unsigned long long ans = count - cumCount;
  //   cout << "Num positions at depth=" << depth << " is " << ans << "\n";
  //   cout << "Num positions up to depth=" << depth << " is " << count << "\n";
  //   assert(ans == ANS[depth]);
  //   cumCount = count;
  // }
#if FLIP_OPT
  unordered_map<Key, ull, KeyHasher> memo;
#else
  // unordered_set<Key, KeyHasher> memo;
  Memo memo;
#endif
  cout << "starting!\n";
  // unordered_set<Key2> memo;
  unsigned long long count = 0;
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
  // cout << "hashing took " << hashTime << "ms\n";
  // cout << "makeMove() took " << moveTime << "ms\n";


  cout << "Num positions up to depth=" << maxDepth << " is " << count << "\n";
  unsigned long long ans = 0;
  for (int i = 0; i <= maxDepth; i++) {
    ans += ANS[i];
  }
  if (ans != count) {
    cout << "Didn't match official answer: " << ans << "\n";
  } else {
    cout << "PASSED\n";
  }
}
