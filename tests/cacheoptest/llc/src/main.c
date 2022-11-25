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
  test_invalid();

  asm("fence\n");
  test_clean();

  asm("fence\n");
  test_flush();


  /*  test region operation */
  asm("fence\n");
  make_region_invalid((uint64_t)test_buffer[32], 256);
  for(int i = 32; i < TEST_BUFFER_SIZE/4; ++i) {
    uint64_t test_data = test_buffer[i];
    if(test_data == i + 1) {
      printf("test failed, test_buffer[%d] = %lld, original: %d\n", i, test_buffer[i], i+1);
      _halt(-1);
    }
  }
  printf("make region invalid succeed!\n\n");

  asm("fence\n");
  make_region_clean((uint64_t)test_buffer[64], 256);
  for(int i = 64; i < TEST_BUFFER_SIZE/4; ++i) {
    uint64_t test_data = test_buffer[i];
    if(test_data != i + 1) {
      printf("test failed, test_buffer[%d] = %lld, original: %d\n", i, test_buffer[i], i+1);
      _halt(-1);
    }
  }
  printf("make region clean succeed!\n\n");

  asm("fence\n");
  make_region_flush((uint64_t)test_buffer[96], 256);
  for(int i = 96; i < TEST_BUFFER_SIZE/4; ++i) {
    uint64_t test_data = test_buffer[i];
    if(test_data != i + 1) {
      printf("test failed, test_buffer[%d] = %lld, original: %d\n", i, test_buffer[i], i+1);
      _halt(-1);
    }
  }
  printf("make region flush succeed!\n");

  return 0;
}

void test_invalid() {
  if(is_ready()) {
    make_invalid((uint64_t)&test_buffer[0]);
  } else {
    printf("CtrlUnit is not ready yet.");
    _halt(1);
  }
  wait(100);
  for(int i = 0; i < 8; ++i) {
    if (test_buffer[i] == i+1) {  
      // with diff-test, NEMU will report Refill failed!
      printf("CMO_INV failed: test_buffer[%d] = %lld, original: %d\n", i, test_buffer[i], i+1);
      _halt(1);
    }
  }
  printf("CMO_INV succeed!\n\n");
}

void test_clean() {
  if(is_ready()) {
    make_clean((uint64_t)&test_buffer[8]);
  } else {
    printf("CtrlUnit is not ready yet.");
    _halt(1);
  }
  wait(100);
  for(int i = 8; i < 16; ++i) {
    if (test_buffer[i] != i+1) {  
      printf("CMO_CLEAN failed: test_buffer[%d] = %lld, original: %d\n", i, test_buffer[i], i+1);
      _halt(1);
    }
  }
  printf("CMO_CLEAN succeed!\n\n");
}

void test_flush() {
  if(is_ready()) {
    make_flush((uint64_t)&test_buffer[16]);
  } else {
    printf("CtrlUnit is not ready yet.");
    _halt(1);
  }
  wait(100);
  for(int i = 16; i < 24; ++i) {
    if (test_buffer[i] != i+1) {  
      printf("CMO_FLUSH failed: test_buffer[%d] = %lld, original: %d\n", i, test_buffer[i], i+1);
      _halt(1);
    }
  }
  printf("CMO_FLUSH succeed!\n\n");
}

