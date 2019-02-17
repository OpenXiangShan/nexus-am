#include <stdlib.h>
#include <klib.h>

#define HEAP_SIZE (32 * 1024 * 1024)

class _Init {
  public : _Init() {
    _heap.start = malloc(HEAP_SIZE);
    _heap.end = (uint8_t *)_heap.start + HEAP_SIZE;
  }
  public : ~_Init() {
    free(_heap.start);
  }
};

static _Init _init_platform;

extern "C" {
// This dummy function will be called in trm.c.
// The purpose of this dummy function is to let linker add this file to the object
// file set. Without it, the constructor of @_init_platform will not be linked.
void platform_dummy() {
}

}
