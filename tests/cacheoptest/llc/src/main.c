// #include <stdio.h>
#include "huancunop.h"

#define TEST_BUFFER_SIZE 128

volatile uint64_t test_buffer[TEST_BUFFER_SIZE] __attribute__((aligned(64))) = {0};

void test_invalid();

int main() {
  printf("HuanCun op (mmio based) test.\n");
  printf("l3 size -- %d KB\nnr_way -- %d\nnr_bank -- %d\nnr_set -- %d\n", L3_SIZE_KB, L3_NR_WAY, L3_NR_BANK, SET_SIZE);
  
  /* Fill data for test_buffer */
  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    test_buffer[i] = i + 1;
  }

  asm("fence\n");

  test_invalid();

  return 0;
}

// just a test example.
void test_invalid() {
  if(is_ready()) {
    make_invalid((uint64_t)&test_buffer[0]);
  } else {
    printf("CtrlUnit is not ready yet.");
    _halt(1);
  }
  wait(100);
  if (test_buffer[0] == 1) {  
    // with diff-test, NEMU will report Refill failed!
    printf("CMO_INV failed: test_buffer[0] = %lx", 1, test_buffer[0]);
  } else {
    printf("CMO_INV succeed!");
    _halt(0);
  }
}

