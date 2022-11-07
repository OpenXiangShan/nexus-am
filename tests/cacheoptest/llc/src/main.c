#include "huancunop.h"

#define TEST_BUFFER_SIZE 128

volatile uint64_t test_buffer[TEST_BUFFER_SIZE] __attribute__((aligned(64))) = {0};

void test_invalid();
void test_clean();
void test_flush();

int main() {
  printf("HuanCun op (mmio based) test.\n");
  // printf("l3 size -- %d KB\nnr_way -- %d\nnr_bank -- %d\nnr_set -- %d\n", L3_SIZE_KB, L3_NR_WAY, L3_NR_BANK, SET_SIZE);
  
  /* Fill data for test_buffer */
  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    test_buffer[i] = i + 1;
  }

  asm("fence\n");

  /*  simple test  */
  // test_flush();

  /*  test region operation */
  // make_region_flush((uint64_t)test_buffer, 512);
  // for(int i = 0; i < TEST_BUFFER_SIZE; ++i) {
  //   uint64_t test_data = test_buffer[i];
  //   if(test_data != i + 1) {
  //     printf("test failed, test_buffer[%d] = %d, original: %d\n", i, test_buffer[i], i+1);
  //   }
  // }

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
    printf("CMO_INV failed: test_buffer[0] = %lx, original: %d\n", test_buffer[0], 1);
    _halt(1);
  } else {
    printf("CMO_INV succeed!");
    _halt(0);
  }
}

void test_clean() {
  if(is_ready()) {
    make_clean((uint64_t)&test_buffer[1]);
  } else {
    printf("CtrlUnit is not ready yet.");
    _halt(1);
  }
  wait(100);
  if (test_buffer[1] != 2) {  
    printf("CMO_CLEAN failed: test_buffer[1] = %lx, original: %d\n", test_buffer[1], 2);
    _halt(1);
  } else {
    printf("CMO_CLEAN succeed!");
    _halt(0);
  }
}

void test_flush() {
  if(is_ready()) {
    make_flush((uint64_t)&test_buffer[2]);
  } else {
    printf("CtrlUnit is not ready yet.");
    _halt(1);
  }
  wait(100);
  if (test_buffer[2] != 3) {  
    printf("CMO_FLUSH failed: test_buffer[0] = %lx, original: %d\n", test_buffer[2], 3);
    _halt(1);
  } else {
    printf("CMO_FLUSH succeed!");
    _halt(0);
  }
}

