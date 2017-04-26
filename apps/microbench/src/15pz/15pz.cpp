#include <benchmark.h>
#include "puzzle.h"
#include "heap.h"

#define N 4

extern "C" {

void bench_15pz_prepare() {
  srand(6667);
}

void bench_15pz_run() {
  N_puzzle<N> puzzle;
  while (!puzzle.solvable()) {
    puzzle = N_puzzle<N>();
  }

  auto *heap = new Updatable_heap<N_puzzle<N>, 1024>();
  heap->push( puzzle, 0 );

  int n = 0;

  while( heap->size() != 0 && n != 1000000000 ) {
    N_puzzle<N> top = heap->pop();
    ++n;

    if ( top == N_puzzle<N>::solution() ) {
      // We are done
      printk("path length: %d\n", heap->length(top));
      printk("maximum heap size: %d\n", heap->maximum_size());
      printk("vertices: %d\n", n);
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
  return (const char *)NULL;
}

}

