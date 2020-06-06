#include "klib.h"

extern uint32_t _heap_start;

// step unit: byte
void flush(const char *name, uint32_t *start, uint32_t *end, int step) {
  volatile uint32_t *p;
  int s = step / sizeof(uint32_t);

  printf("Testing '%s', interval = [%x, %x), step = %d\n", name,
      (uint32_t)(uintptr_t)start, (uint32_t)(uintptr_t)end, step);
  printf("Writing sequence...\n");

  for (p = start; p < end; p += s) {
    uint32_t data = (uint32_t)(uintptr_t)p;
    if ((data & 0xffff) == 0) {
      printf("finish writing address %x with data %x...\n", data, data);
    }
    *p = data;
  }

  printf("Reading sequence and check...\n");

  for (p = start; p < end; p += s) {
    uint32_t data = *p;
    uint32_t right = (uint32_t)(uintptr_t)p;
    if ((right & 0xffff) == 0) {
      printf("finish checking address %x...\n", right);
    }
    if (data != right) {
      printf("[ERROR] checking address %x fail: right = %x, wrong = %x...\n", right, right, data);
    }
    assert(data == right);
  }

  printf("SDRAM test '%s' pass!!!\n", name);
}

#define L2_SIZE (128 * 1024)
#define L2_WAY 4
#define L2_SET (L2_SIZE / 64 / L2_WAY)

int main() {
  uint32_t *start = _heap.start;
  flush("set", start, start + (L2_SIZE * 2) / sizeof(uint32_t), 64 * L2_SET);
  flush("whole sdram", start, _heap.end, sizeof(uint32_t));

  return 0;
}
