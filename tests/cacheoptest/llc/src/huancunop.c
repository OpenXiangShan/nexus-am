#include <am.h>
#include <klib.h>

// naive l2 cache op test

#define CACHE_CTRL_BASE 0x39000100
#define CACHE_CMD_BASE 0x39000200
#define HART_CTRL_RESET_REG_BASE 0x39001000
#define CMD_CMO_INV (0 + 16)
#define CMD_CMO_CLEAN (1 + 16)
#define CMD_CMO_FLUSH (2 + 16)
#define CTRL_TAG_OFFSET 0
#define CTRL_SET_OFFSET 8
#define CTRL_WAY_OFFSET 16
#define CTRL_DATA_OFFSET 24
#define CTRL_DIR_OFFSET 32
#define TEST_BUFFER_SIZE 32
#define L3_SIZE_KB (6 * 1024)

volatile uint64_t test_buffer[TEST_BUFFER_SIZE] = {0};

void success() {
  printf("test passed.\n");
  asm("li a0, 0\n");
  asm(".word 0x0000006b\n");
}

void failure() {
  printf("test failed.\n");
  asm("li a0, 1\n");
  asm(".word 0x0000006b\n");
}

int main(){
  printf("huancun op (mmio based) test\n");
  printf("huancun l3 size is set to %d KB\n", L3_SIZE_KB);
  for(int i = 0; i < TEST_BUFFER_SIZE; i++){
    test_buffer[i] = (uint64_t)&test_buffer[i];
  }
  uint64_t tag = (uint64_t)&test_buffer >> 6 >> 12; // paddr to l3 tag
  uint64_t set = (uint64_t)&test_buffer >> 6; // paddr to l3 set
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = tag;
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = set;
  printf("addr %x tag %x set %x\n", &test_buffer, tag, set);
  asm("fence\n");

  // (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_FLUSH;
  // printf("huancun op flush done\n");
  // printf("data %lx\n", test_buffer[0]);
  // (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_CLEAN;
  // printf("huancun op clean done\n");
  // printf("data %lx\n", test_buffer[0]);
  // (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_INV;
  // printf("huancun op invalid done\n");
  // printf("data %lx\n", test_buffer[0]);

  test_buffer[0] = 1;
  asm("fence\n");
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_INV;
  printf("huancun op invalid done\n");
  printf("data %lx\n", test_buffer[0]);

  test_buffer[0] = 2;
  asm("fence\n");
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_CLEAN;
  printf("huancun op clean done\n");
  printf("data %lx\n", test_buffer[0]);

  test_buffer[0] = 3;
  asm("fence\n");
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_FLUSH;
  printf("huancun op flush done\n");
  printf("data %lx\n", test_buffer[0]);

  // test to be added for a wider addr range

  success();
  return 0;
}
