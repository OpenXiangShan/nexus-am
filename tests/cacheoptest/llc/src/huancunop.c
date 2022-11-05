#include <am.h>
#include <klib.h>

// naive LLC cache op test
#if defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
#define CACHE_CTRL_BASE 0x1f10040100
#define CACHE_CMD_BASE 0x1f10040200
#define HART_CTRL_RESET_REG_BASE 0x1f10001000
#else
#define CACHE_CTRL_BASE 0x39000100
#define CACHE_CMD_BASE 0x39000200
#define HART_CTRL_RESET_REG_BASE 0x39001000
#endif
#define CMD_CMO_INV (0 + 16)
#define CMD_CMO_CLEAN (1 + 16)
#define CMD_CMO_FLUSH (2 + 16)
#define CTRL_TAG_OFFSET 0
#define CTRL_SET_OFFSET 8
#define CTRL_WAY_OFFSET 16
#define CTRL_DATA_OFFSET 24
#define CTRL_DIR_OFFSET 32
#define TEST_BUFFER_SIZE 128
#define L3_SIZE_KB (2 * 1024)
#define L3_NR_WAY 8
#define L3_NR_BANK 4
#define OFFSET_LEN 6
#define CACHE_LINE_SIZE_BIT 512
#define CACHE_LINE_SIZE_BYTE (CACHE_LINE_SIZE_BIT / 8)

unsigned int log2(unsigned int n) {
  unsigned int result = 0;
  while (n > 1) {
    assert(n % 2 == 0);
    n = n / 2;
    result++;
  }
  return result;
}

void wait(int cycle) {
  volatile int i = cycle;
  while(i > 0){
    i--;
  }
}

volatile uint64_t test_buffer[TEST_BUFFER_SIZE] __attribute__((aligned(64))) = {0};

void success() {
  printf("test passed.\n");
  // asm("li a0, 0\n");
  // asm(".word 0x0000006b\n");
  _halt(0);
}

void failure() {
  printf("test failed.\n");
  // asm("li a0, 1\n");
  // asm(".word 0x0000006b\n");
  _halt(1);
}

void test1() {
  test_buffer[0] = 1;
  asm("fence\n");
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_INV;
  wait(100);
  printf("huancun op invalid done\n");
  printf("data %lx\n", test_buffer[0]);
  if (test_buffer[0] == 1) {
    printf("CMO_INV failed: data right %lx wrong %lx", 1, test_buffer[0]);
    failure();
  }
}

void test2() {
  test_buffer[0] = 2;
  asm("fence\n");
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_CLEAN;
  wait(100);
  printf("huancun op clean done\n");
  printf("data %lx\n", test_buffer[0]);
  if (test_buffer[0] != 2) {
    printf("CMD_CMO_CLEAN failed: data right %lx wrong %lx", 2, test_buffer[0]);
    failure();
  }
}

void test3() {
  test_buffer[0] = 3;
  asm("fence\n");
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_FLUSH;
  wait(100);
  printf("huancun op flush done\n");
  printf("data %lx\n", test_buffer[0]);
  if (test_buffer[0] != 3) {
    printf("CMD_CMO_FLUSH failed: data right %lx wrong %lx", 3, test_buffer[0]);
    failure();
  }
}

// Flush a cacheline (512 bit) to memory
void flush_to_memory(uint64_t paddr) {
  // printf("l3 size is set to %d KB, nr_way is set to %d, nr_bank is set to %d, ", L3_SIZE_KB, L3_NR_WAY, L3_NR_BANK);
  unsigned int set_size = L3_SIZE_KB * 1024 / L3_NR_BANK / L3_NR_WAY / 64;
  unsigned int set_len = log2(set_size);
  // printf("nr_set is %u, set_len is %u\n", set_size, set_len);

  /* In our LLC design, full address is passed by CtrlUnit to one of the SliceCtrls according to BankBits
   * Afterwards, BankBits are truncated in SliceCtrl to generate real MSHR request
   * So we should provide full address here
   */
  uint64_t tag = (paddr >> OFFSET_LEN) >> set_len; // paddr to l3 tag
  uint64_t set = (paddr >> OFFSET_LEN) & (set_size-1); // paddr to l3 set
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = tag;
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = set;
  // printf("flush to memory: addr 0x%llx tag 0x%llx set 0x%llx\n", &test_buffer, tag, set);
  asm("fence\n");
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_CLEAN; // or CMD_CMO_FLUSH
  wait(100);
}

// Flush an n*512 bit address region to memory
void flush_region_to_memory(uint64_t start_paddr, uint64_t size_in_byte) {
  // pre-calcuated const
  unsigned int set_size = L3_SIZE_KB * 1024 / L3_NR_BANK / L3_NR_WAY / 64;
  unsigned int set_len = log2(set_size);
  // printf("l3 size is set to %d KB, nr_way is set to %d, nr_bank is set to %d, ", L3_SIZE_KB, L3_NR_WAY, L3_NR_BANK);
  // printf("nr_set is %u, set_len is %u\n", set_size, set_len);

  // flush sq and sbuffer
  asm("fence\n");

  // send l3 cache flush op to l3 cache controller
  for(uint64_t current_paddr = start_paddr; current_paddr < (start_paddr + size_in_byte); current_paddr += CACHE_LINE_SIZE_BYTE){
    uint64_t tag = (current_paddr >> OFFSET_LEN) >> set_len; // paddr to l3 tag
    uint64_t set = (current_paddr >> OFFSET_LEN) & (set_size-1); // paddr to l3 set
    *(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = tag;
    *(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = set;
    // printf("flush to memory: addr 0x%llx tag 0x%llx set 0x%llx\n", &test_buffer, tag, set);
    (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_CLEAN; // or CMD_CMO_FLUSH
  }

  // wait for the last cache op to finish
  wait(100);
}

int main() {
  printf("HuanCun op (mmio based) test. Note that --no-diff is required!\n");
  printf("HuanCun l3 size is set to %d KB, nr_way is set to %d, nr_bank is set to %d, ", L3_SIZE_KB, L3_NR_WAY, L3_NR_BANK);
  unsigned int set_size = L3_SIZE_KB * 1024 / L3_NR_BANK / L3_NR_WAY / 64;
  unsigned int set_len = log2(set_size);
  printf("nr_set is %u, set_len is %u\n", set_size, set_len);
  
  /* Fill data for test_buffer */
  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    test_buffer[i] = (uint64_t)&test_buffer[i];
  }

  /* In our LLC design, full address is passed by CtrlUnit to one of the SliceCtrls according to BankBits
   * Afterwards, BankBits are truncated in SliceCtrl to generate real MSHR request
   * So we should provide full address here
   */
  uint64_t tag = ((uint64_t)&test_buffer >> OFFSET_LEN) >> set_len; // paddr to l3 tag
  uint64_t set = ((uint64_t)&test_buffer >> OFFSET_LEN) & (set_size-1); // paddr to l3 set
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = tag;
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = set;
  printf("addr 0x%llx tag 0x%llx set 0x%llx\n", &test_buffer, tag, set);
  asm("fence\n");

  test1();
  test2();
  test3();

  // test to be added for a wider addr range
  success();
  return 0;
}
