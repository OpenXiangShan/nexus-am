#include <am.h>
#include <klib.h>
#include <stdint.h>

#ifndef STR_BYTES
#define STR_BYTES 0UL
#endif

#ifndef TIMES
#define TIMES 1UL
#endif

#if TIMES < 1
#error "TIMES must be at least 1"
#endif

#define BUFFER_ALIGNMENT 64UL

#if (defined(TEST_strcpy) + defined(TEST_strlen) + \
     defined(TEST_strcmp) + defined(TEST_strchr)) != 1
#error "Define exactly one string test"
#endif

#if defined(TEST_strcpy)
extern char *vector_strcpy(char *dst, const char *src);
#elif defined(TEST_strlen)
extern uintptr_t vector_strlen(const char *s);
#elif defined(TEST_strcmp)
extern int vector_strcmp(const char *s1, const char *s2);
#elif defined(TEST_strchr)
extern char *vector_strchr(const char *s, int c);
#endif
extern void vector_fill_bytes(uint8_t *dst, uintptr_t length);

static inline uintptr_t read_mcycle(void) {
  uintptr_t value;
  asm volatile("csrr %0, mcycle" : "=r"(value) : : "memory");
  return value;
}

static uintptr_t vector_vlmax_e8_m8(void) {
  uintptr_t vlmax;
  asm volatile("li t0, -1\n"
               "vsetvli %0, t0, e8, m8, ta, ma"
      : "=r"(vlmax) : : "t0");
  return vlmax;
}

static void init_vector(void) {
  asm volatile(
      "lui a0, 0x2\n"
      "addiw a0, a0, 512\n"
      "csrs mstatus, a0\n"
      "csrwi vcsr, 0\n"
      "csrwi vstart, 0"
      : : : "a0", "memory");
}

static uintptr_t align_up(uintptr_t value, uintptr_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

static int allocate_buffers(uintptr_t vlmax, uint8_t **src,
    uint8_t **work) {
  uintptr_t read_bytes = (uintptr_t)STR_BYTES + vlmax;
  uintptr_t src_addr = align_up((uintptr_t)_heap.start, BUFFER_ALIGNMENT);
  uintptr_t end_addr = src_addr + read_bytes;

  if (vlmax == 0 || read_bytes < (uintptr_t)STR_BYTES ||
      end_addr < src_addr) {
    return 0;
  }

#if defined(TEST_strcmp) || defined(TEST_strcpy)
  uintptr_t work_addr = align_up(end_addr, BUFFER_ALIGNMENT);
#if defined(TEST_strcmp)
  const uintptr_t work_bytes = read_bytes;
#else
  const uintptr_t work_bytes = (uintptr_t)STR_BYTES + 1;
#endif
  if (work_addr < end_addr || work_bytes < (uintptr_t)STR_BYTES) {
    return 0;
  }
  end_addr = work_addr + work_bytes;
  if (end_addr < work_addr) {
    return 0;
  }
#endif

  if (end_addr > (uintptr_t)_heap.end) {
    return 0;
  }

  *src = (uint8_t *)src_addr;
  *work = NULL;
#if defined(TEST_strcmp) || defined(TEST_strcpy)
  *work = (uint8_t *)work_addr;
#endif
  return 1;
}

static void print_perf(const char *name, uintptr_t cycles) {
  uintptr_t bytes_per_cycle_x100 = 0;
  uintptr_t total_bytes = (uintptr_t)STR_BYTES * (uintptr_t)TIMES;
  if (cycles != 0) {
    bytes_per_cycle_x100 =
        (total_bytes * 100UL + cycles / 2) / cycles;
  }

  printf("%s: bytes=%lu calls=%lu cycles=%lu bytes/cycle=%lu.%02lu\n",
      name, (unsigned long)STR_BYTES, (unsigned long)TIMES,
      (unsigned long)cycles,
      (unsigned long)(bytes_per_cycle_x100 / 100UL),
      (unsigned long)(bytes_per_cycle_x100 % 100UL));
}

int main(void) {
  uint8_t *src;
  uint8_t *work;

  init_vector();
  uintptr_t vlmax = vector_vlmax_e8_m8();
  if (!allocate_buffers(vlmax, &src, &work)) {
    printf("rvv_string_bench failed: bytes=%lu vlmax=%lu\n",
        (unsigned long)STR_BYTES, (unsigned long)vlmax);
    return 1;
  }

  vector_fill_bytes(src, (uintptr_t)STR_BYTES);
  src[STR_BYTES] = 0;

#if defined(TEST_strcmp)
  vector_fill_bytes(work, (uintptr_t)STR_BYTES);
  work[STR_BYTES] = 0;
#endif

  uintptr_t start = read_mcycle();

  for (uintptr_t i = 0; i < (uintptr_t)TIMES; i++) {
#if defined(TEST_strcpy)
    vector_strcpy((char *)work, (const char *)src);
#elif defined(TEST_strlen)
    vector_strlen((const char *)src);
#elif defined(TEST_strcmp)
    vector_strcmp((const char *)src, (const char *)work);
#elif defined(TEST_strchr)
    vector_strchr((const char *)src, 0);
#endif
  }

  uintptr_t cycles = read_mcycle() - start;

#if defined(TEST_strcpy)
  print_perf("vector_strcpy", cycles);
#elif defined(TEST_strlen)
  print_perf("vector_strlen", cycles);
#elif defined(TEST_strcmp)
  print_perf("vector_strcmp", cycles);
#elif defined(TEST_strchr)
  print_perf("vector_strchr", cycles);
#endif

  return 0;
}
