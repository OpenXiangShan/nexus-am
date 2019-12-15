#include "klib.h"

uint64_t array[1024][4][8]; // 256 KB

int main() {
  array[0][0][0] = (uint64_t)&array[0][0][0];
  array[0][1][0] = (uint64_t)&array[0][1][0];
  array[0][2][0] = (uint64_t)&array[0][2][0];
  array[0][3][0] = (uint64_t)&array[0][3][0];
  array[512][0][0] = (uint64_t)&array[512][0][0];
  array[512][1][0] = (uint64_t)&array[512][1][0];
  array[512][2][0] = (uint64_t)&array[512][2][0];
  array[512][3][0] = (uint64_t)&array[512][3][0];

  assert(array[0][0][0] == (uint64_t)&array[0][0][0]);
  assert(array[0][1][0] == (uint64_t)&array[0][1][0]);
  assert(array[0][2][0] == (uint64_t)&array[0][2][0]);
  assert(array[0][3][0] == (uint64_t)&array[0][3][0]);
  assert(array[512][0][0] == (uint64_t)&array[512][0][0]);
  assert(array[512][1][0] == (uint64_t)&array[512][1][0]);
  assert(array[512][2][0] == (uint64_t)&array[512][2][0]);
  assert(array[512][3][0] == (uint64_t)&array[512][3][0]);

  return 0;

  uint32_t *p;
  uint32_t *p_start = (uint32_t *)(uintptr_t)(0x80040000); // _heap.end
  uint32_t *p_end = (uint32_t *)(uintptr_t)(0x80080000); // _heap.end

  printf("Testing SDRAM interval [%x, %x)...\n",
      (uint32_t)(uintptr_t)p_start, (uint32_t)(uintptr_t)p_end);
  printf("Writing sequence...\n");

  for (p = p_start; p < p_end; p ++) {
    uint32_t data = (uint32_t)(uintptr_t)p;
    //if ((data & 0xfffff) == 0) {
    //  printf("finish writing address %x with data %x...\n", data, data);
    //}
    *p = data;
  }

  printf("Reading sequence and check...\n");

  for (p = p_start; p < p_end; p ++) {
    uint32_t data = *p;
    uint32_t right = (uint32_t)(uintptr_t)p;
    //if ((right & 0xfffff) == 0) {
    //  printf("finish checking address %x...\n", right);
    //}
    if (data != right) {
      printf("[ERROR] checking address %x fail: right = %x, wrong = %x...\n", right, right, data);
    }
    assert(data == right);
  }

  printf("SDRAM test pass!!!\n");
  return 0;
}
