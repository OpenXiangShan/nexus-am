#include <amtest.h>
#include <xs.h>

enum {
  s_idle,
  s_read,
  s_write,
  s_wait_resp_b,
  s_wait_resp_r0,
  s_wait_resp_r1
};

typedef struct __attribute__((__packed__)) {
  uint64_t value    : 8;
  uint64_t reserved : 56;
} dma_state;

typedef struct {
  uint64_t data[8];
  dma_state state;
  uint64_t address;
  uint64_t mask;
  uint64_t reserved[5];
} dma_mshr;

// CHANGE this according to the memory map
volatile dma_mshr *mshr = (volatile dma_mshr *)0x1f00070000UL;
volatile uint64_t *mshr_valid = (volatile uint64_t *)0x1f00072000UL;

// 4GB - 6GB
volatile uint8_t *memory = (volatile uint8_t *)0x2100000000UL;
// 6GB - 8GB
volatile uint8_t *ref_memory = (volatile uint8_t *)0x2180000000UL;

inline void riscv_fence() {
  asm volatile("fence");
}

uint64_t random_number() {
  static uint64_t count = 1;
  count++;
  uint64_t x = ((count >> 16) ^ count) * 0x45d9f3b;
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = (x >> 16) ^ x;
  return x;
}

inline uint64_t random_memory_offset() {
  // max 16MB
  return random_number() % 0x1000000UL;
}

void dma_test() {
  assert(sizeof(dma_state) == 8);
  assert(sizeof(dma_mshr) == 16 * 8);
  printf("Starting DMA Test\n");
  printf("Setting memory\n");
  // randomly touch 256K * 64B = 8MB memory
  for (int i = 0; i < 64; i++) {
    uint64_t offset = random_memory_offset();
    uint64_t rand_num = random_number();
    // fetch memory to cache hierarchy randomly
    if (rand_num & 1) {
      memory[offset] = random_number();
    }
    // memory[offset] = rand_num;
    // ref_memory[offset] = rand_num;
    mshr[i].data[3] = rand_num;
    mshr[i].data[5] = rand_num;
    mshr[i].state.value = s_write;
    uint64_t address = (uint64_t)(memory + offset) ^ ((uint64_t)(memory + offset) & 0x3f);
    mshr[i].address = (uint64_t)(memory + offset) ^ ((uint64_t)(memory + offset) & 0x3f);
    assert(address == mshr[i].address);
    mshr[i].mask = 0xff00ff000000UL;
    volatile uint64_t *t = (volatile uint64_t *)(mshr[i].address + (uint64_t)(ref_memory - memory));
    t[3] = rand_num;
    t[5] = rand_num;
  }
  riscv_fence();
  printf("Finished setting memory. Starting DMA write.\n");
  *mshr_valid = 0xffffffffffffffffUL;
  bool mshr_cleared = false;
  while (!mshr_cleared) {
    mshr_cleared = true;
    for (int i = 0; i < 64; i++) {
      if (mshr[i].state.value) {
        mshr_cleared = false;
        break;
      }
    }
  }
  riscv_fence();
  printf("Finished DMA write. Starting DMA read.\n");
  for (int i = 0; i < 64; i++) {
    uint64_t rand_num = random_number();
    mshr[i].data[3] = rand_num;
    mshr[i].data[5] = rand_num;
    mshr[i].state.value = s_read;
    // do not touch mshr.address/mask
  }
  riscv_fence();
  *mshr_valid = 0xffffffffffffffffUL;
  mshr_cleared = false;
  while (!mshr_cleared) {
    mshr_cleared = true;
    for (int i = 0; i < 64; i++) {
      if (mshr[i].state.value) {
        mshr_cleared = false;
        break;
      }
    }
  }
  riscv_fence();
  printf("Finished DMA read. Starting CPU read.\n");
  for (int i = 0; i < 64; i++) {
    uint64_t base_offset = (uint64_t *)mshr[i].address - (uint64_t *)memory;
    volatile uint64_t *golden = (uint64_t *)ref_memory + base_offset;
    volatile uint64_t *dut = (uint64_t *)memory + base_offset; 
    for (int j = 0; j < 8; j++) {
      // only difftest for masked written data because original value in memory may be non-zero
      if (j != 3 && j != 5) continue;
      uint64_t dut_data = dut[j];
      uint64_t ref_data = golden[j];
      if (dut_data != ref_data || mshr[i].data[j] != ref_data) {
        printf("[ERROR  ] Test %d at offset %d: DUT(0x%016lx) != REF(0x%016lx) at address 0x%lx and 0x%lx\n",
          i, j, dut_data, ref_data, dut + j, golden + j);
        _halt(1);
      }
      else {
        // printf("[SUCCESS] Test %d at offset %d: DUT(0x%016lx) == REF(0x%016lx) at address 0x%lx and 0x%lx\n",
        //   i, j, dut_data, ref_data, dut + j, golden + j);
      }
    }
  }
  *mshr_valid = 0x0UL;
  printf("All tests passed.\n");
}
