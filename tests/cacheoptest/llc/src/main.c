// main.c

#include "cmotest.h"

#define TEST_BUFFER_SIZE 1024

volatile uint64_t test_buffer[TEST_BUFFER_SIZE] __attribute__((aligned(64))) = {0};

int main() {
  // load datas to cache
  for(int i = 0; i < TEST_BUFFER_SIZE; ++i) {
    *(test_buffer + i) = i;
  }

  asm("fence\n");

  // you can issue CMO by writing address to register CTL_ADDR manually, or use 
  // functions to implement this. please inquire 'ctl_rdy' before issue a CMO.
  while(1) {
    if(cmo_ready()) {
      *(uint64_t*)(CACHE_CTRL_BASE + CTRL_ADDR_OFFSET) = (uint64_t)test_buffer;
      *(uint64_t*)CACHE_CMD_BASE = CMD_CMO_INV;
      break;
    }
  }

  // asm("fence\n");

  // use functions to implement each CMO individually.
  for(volatile int i = 0; i < 2; ++i) {
    while(1) {
      if(cmo_ready()) {
        test_invalid((uint64_t)test_buffer);
        break;
      }
    }
  }

  for(volatile int i = 0; i < 2; ++i) {
    while(1) {
      if(cmo_ready()) {
        test_clean((uint64_t)(test_buffer + 8));
        break;
      }
    }
  }

  for(volatile int i = 0; i < 2; ++i) {
    while(1) {
      if(cmo_ready()) {
        test_flush((uint64_t)(test_buffer + 16));
        break;
      }
    }
  }

  printf("cmotest ends\n");
  return 0;
}

