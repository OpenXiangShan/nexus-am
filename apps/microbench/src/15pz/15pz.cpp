#include <benchmark.h>
#include "puzzle.h"
#include "heap.h"

const int N = 4;
const int ANS = 428940;
const int MAXN = 65536;

int PUZZLE[N*N] = {
  0, 2, 3, 4,
  9, 6, 7, 8,
  5, 11, 10, 12,
  1, 15, 13, 14,
};

int ans;

extern "C" {

void bench_15pz_prepare() {
}

void bench_15pz_run() {
  N_puzzle<N> puzzle(PUZZLE);
  assert(puzzle.solvable());

  auto *heap = new Updatable_heap<N_puzzle<N>, MAXN>();
  heap->push( puzzle, 0 );

  int n = 0;
  ans = -1;

  while( heap->size() != 0 && n != MAXN ) {
    N_puzzle<N> top = heap->pop();
    ++n;

    if ( top == N_puzzle<N>::solution() ) {
      // We are done
      ans = heap->length(top) * n;
      return;
    }

    if ( top.tile_left_possible() ) {
      heap->push( top.tile_left(), heap->length( top ) + 1 );
    }

    if ( top.tile_right_possible() ) {
      heap->push( top.tile_right(), heap->length( top ) + 1 );
    }

    if ( top.tile_up_possible() ) {
      heap->push( top.tile_up(), heap->length( top ) + 1 );
    }

    if ( top.tile_down_possible() ) {
      heap->push( top.tile_down(), heap->length( top ) + 1 );
    }
  }
}


const char * bench_15pz_validate() {
  return (ans == ANS) ? (const char*)NULL : "wrong answer";
}

}

