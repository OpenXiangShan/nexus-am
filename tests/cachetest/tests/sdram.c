#include "klib.h"

extern uint32_t _heap_start;
int main() {
  volatile uint32_t *p;
  uint32_t *p_start = (uint32_t *)(uintptr_t)(0x80100000); // _heap.end
  uint32_t *p_end = (uint32_t *)(uintptr_t)  (0x84000000); // _heap.end

  printf("Testing SDRAM interval [%x, %x)...\n",
      (uint32_t)(uintptr_t)p_start, (uint32_t)(uintptr_t)p_end);
  printf("Writing sequence...\n");

  for (p = p_start; p < p_end; p ++) {
    uint32_t data = (uint32_t)(uintptr_t)p;
    if ((data & 0xffff) == 0) {
      printf("finish writing address %x with data %x...\n", data, data);
    }
    *p = data;
  }

  printf("Reading sequence and check...\n");

  for (p = p_start; p < p_end; p ++) {
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

  printf("SDRAM test pass!!!\n");
  return 0;
}
