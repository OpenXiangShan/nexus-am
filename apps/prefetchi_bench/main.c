// A simple benchmark for the prefetch.i instruction (RISC-V Zicbop) on XiangShan.

// This construct a jump chain over CHAIN_LEN I$ lines,
// every line locates in the same I$ set to cause frequent I$ misses.
// The chain order is data-driven (a table walked by register t2) and reshuffled every pass,
// so branch target prediction -- and hence fdip -- should not able to learn it.
// Then we measure the time to run the chain with and without prefetch.i, and compare the results.

// Use `make ARCH=riscv64-xs` to build and run the benchmark on XiangShan.
// Optionally, you can set the following macros in Makefile to control the benchmark parameters:
//   SEED: the seed for the random number generator (default 1)
//   PASSES: the number of passes to run (default 20)
//   PF_DIST: the prefetch distance in lines (default 2), i.e. prefetch seq[i+PF_DIST] while visiting seq[i]
//   PF_BATCH: prefetch once every PF_BATCH lines (default 1, max 4), i.e. when visiting seq[i]
//             with i % PF_BATCH == 0, prefetch seq[i+PF_DIST .. i+PF_DIST+PF_BATCH-1] at once
//   FILLER_STRIDE: the stride of the filler branches in bytes (default 4)

#include <klib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SEED
#define SEED 1 // srand seed; both variants replay the same shuffle stream
#endif

#ifndef PASSES
#define PASSES 20 // timed passes per variant
#endif
#if PASSES < 1
#error "PASSES must be >= 1"
#endif

#ifndef PF_DIST
#define PF_DIST 2 // prefetch seq[i+PF_DIST] while visiting seq[i]
#endif
#if PF_DIST < 1 || PF_DIST > 8
#error "PF_DIST must be in [1, 8]"
#endif

#ifndef PF_BATCH
#define PF_BATCH 1 // prefetch once every PF_BATCH lines, covering PF_DIST..PF_DIST+PF_BATCH-1
#endif
#if PF_BATCH < 1 || PF_BATCH > 4
#error "PF_BATCH must be in [1, 4]"
#endif

#ifndef FILLER_STRIDE
#define FILLER_STRIDE 4 // filler branch offset in bytes (4 or 8)
#endif
#if FILLER_STRIDE != 4 && FILLER_STRIDE != 8
#error "FILLER_STRIDE must be 4 or 8"
#endif

#define BASE_ADDR 0x82000000ull
#define CHAIN_LEN 128 // jumps per pass

// I$ parameters aligned with XiangShan kunminghu-v2/v3
#define ICACHE_LINE_SIZE 64 // bytes
#define ICACHE_ASSOC 4 // way
#define ICACHE_SETS 256 // sets

// consecutive areas differ in addr bit 14+, i.e. same I$ set index, new tag
#define LINE_ADDR(a) (BASE_ADDR + (uintptr_t)(a) * ICACHE_SETS * ICACHE_LINE_SIZE)
#define TRAMP_ADDR (BASE_ADDR + (uintptr_t)CHAIN_LEN * ICACHE_SETS * ICACHE_LINE_SIZE) // return trampoline line

// instruction slots per line (each instruction is 4 bytes, no RVC here)
#define SLOTS (ICACHE_LINE_SIZE / 4)

// max prefetch distance (in lines)
#define MAX_PF_DIST 8

// max prefetch batch (in lines)
#define MAX_PF_BATCH 4

// hand-encoded 32-bit instruction words (MARCH has no zicbop; all verified
// against riscv64-linux-gnu-gcc/objdump output)
#define W_LD_T0_0_T2   0x0003b283u // ld   t0, 0(t2)
#define W_ADDI_T2_T2_8 0x00838393u // addi t2, t2, 8
#define W_LD_T1_0_T2   0x0003b303u // ld   t1, imm(t2)
#define W_PREFETCH_I   0x00036013u // prefetch.i 0(t1)
#define W_BEQ_4        0x00000263u // beq  x0, x0, +4
#define W_BEQ_8        0x00000463u // beq  x0, x0, +8
#define W_NOP          0x00000013u
#define W_JR_T0        0x00028067u // jalr x0, 0(t0)
#define W_RET          0x00008067u // jalr x0, 0(ra)

#define W_LD_T1_IMM_T2(imm) (W_LD_T1_0_T2 | (imm) << 20) // ld t1, imm(t2)
#define W_ADDI_T4_T4(imm) ((((imm) & 0xfff) << 20) | 0x000e8e93u) // addi t4, t4, imm
#define W_BGEZ_T4(imm) /* bge t4, x0, +imm */ \
  ((((imm) & 0x1000) << 19) | (((imm) & 0x7e0) << 20) | (29 << 15) | (5 << 12) | \
   (((imm) & 0x1e) << 7) | (((imm) & 0x800) >> 4) | 0x63u)

// farthest seq entry ever read: seq[CHAIN_LEN-1 + PF_DIST + PF_BATCH - 1]
#define SEQ_TAIL (MAX_PF_DIST + MAX_PF_BATCH)

static uintptr_t seq[CHAIN_LEN + SEQ_TAIL + 1];

#define RESULT_PERCISION 6
#define RESULT_SCALE 1000000 // must be 10^RESULT_PERCISION

#define STR2(x) #x
#define STR(x) STR2(x)

void init_area(bool with_pf) {
  // construct chain
  for (int i = 0; i < CHAIN_LEN; i++) {
    uint32_t *p = (uint32_t *)LINE_ADDR(i);
    int s = 0;
    // when entering current cacheline, t2 points to the next line's address, i.e. t2 = &seq[i+1]
    p[s++] = W_LD_T0_0_T2; // t0 = *t2 => t0 = seq[i+1]
#if PF_BATCH == 1
    p[s++] = W_LD_T1_IMM_T2(8 * (PF_DIST - 1)); // t1 = *(t2 + 8*(PF_DIST-1)) => t1 = &seq[i+PF_DIST]
    p[s++] = W_ADDI_T2_T2_8;                    // t2 += 8                    => i++
    p[s++] = with_pf ? W_PREFETCH_I : W_NOP;    // prefetch seq[i+PF_DIST]
#else
    // prefetch once every PF_BATCH lines: t4 is a countdown reset by start_chain();
    // when it goes negative, batch-prefetch seq[i+PF_DIST .. i+PF_DIST+PF_BATCH-1] and reload;
    // otherwise jump 2*PF_BATCH+2 slots ahead (skip to i++)
    p[s++] = W_ADDI_T4_T4(-1);
    p[s++] = W_BGEZ_T4(4 * (2 * PF_BATCH + 2));
    for (int k = 0; k < PF_BATCH; k++) {
      p[s++] = W_LD_T1_IMM_T2(8 * (PF_DIST - 1 + k)); // t1 = &seq[i+PF_DIST+k]
      p[s++] = with_pf ? W_PREFETCH_I : W_NOP;        // prefetch seq[i+PF_DIST+k]
    }
    p[s++] = W_ADDI_T4_T4(PF_BATCH); // reload countdown (t4 is -1 here)
    p[s++] = W_ADDI_T2_T2_8;         // t2 += 8 => i++
#endif
    // fill the rest of the line with BPU-polluting branches
    while (s < SLOTS - 1) {
#if FILLER_STRIDE == 8
      if (s + 1 < SLOTS - 1) {
        p[s++] = W_BEQ_8;
        p[s++] = W_NOP;
      } else {
        p[s++] = W_BEQ_4;
      }
#else
      p[s++] = W_BEQ_4;
#endif
    }
    // sanity check: s should not greater than SLOTS-1
    assert(s == SLOTS - 1);
    // go to next cacheline, i.e. jump to seq[i+1] (t0)
    p[SLOTS - 1] = W_JR_T0;
  }
  // construct final trampoline line that returns to start_chain()
  uint32_t *t = (uint32_t *)TRAMP_ADDR;
  for (int i = 0; i < SLOTS; i++)
    t[i] = W_RET;
  // flush the I$
  asm volatile("fence.i" ::: "memory");
}

void init_seq() {
  for (int i = 0; i < CHAIN_LEN; i++)
    seq[i] = LINE_ADDR(i);
  for (int i = CHAIN_LEN; i <= CHAIN_LEN + SEQ_TAIL; i++)
    seq[i] = TRAMP_ADDR;
}

void shuffle_seq() {
  for (int i = CHAIN_LEN - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    uintptr_t t = seq[i]; seq[i] = seq[j]; seq[j] = t;
  }
}

static inline void start_chain() {
  // loads the address of seq[1] into t2, then jumps to seq[0] (the first line of the chain)
  // the chain will eventually return to this function via the trampoline line
  // t4 is the prefetch countdown used when PF_BATCH > 1
  asm volatile(
    "mv t2, %0\n\t"
    "li t4, 0\n\t"
    "jalr ra, 0(%1)\n\t"
    :
    : "r"(&seq[1]), "r"(seq[0])
    : "t0", "t1", "t2", "t4", "ra", "memory"
  );
}

static inline uint64_t read_mcycle() {
  uint64_t v;
  asm volatile("csrr %0, mcycle" : "=r"(v));
  return v;
}

uint64_t run_passes(int passes) {
  uint64_t start = read_mcycle();
  for (int p = 0; p < passes; p++) {
    shuffle_seq();
    start_chain();
    _putc('.');
  }
  return read_mcycle() - start;
}

uint64_t benchmark(bool with_pf) {
  printf("  init");
  srand(SEED);
  init_area(with_pf);
  init_seq();
  printf("\n  warmup");
  run_passes(2);
  printf("\n  speed");
  return run_passes(PASSES);
}

int main() {
  printf(
    "seed=%d pf_dist=%d pf_batch=%d filler_stride=%d passes=%d chain_len=%d\n",
    SEED, PF_DIST, PF_BATCH, FILLER_STRIDE, PASSES, CHAIN_LEN
  );

  printf("\nrunning baseline (no prefetch.i)...\n");
  uint64_t baseline = benchmark(false);
  printf("\nrunning prefetch (with prefetch.i)...\n");
  uint64_t prefetch = benchmark(true);

  uint64_t jumps = PASSES * CHAIN_LEN;
  printf("\nPASSED\n");
  printf("  baseline: %llu cycles, %llu cycles/jump (%llu jumps)\n", baseline, baseline / jumps, jumps);
  printf("  prefetch: %llu cycles, %llu cycles/jump (%llu jumps)\n", prefetch, prefetch / jumps, jumps);
  uint64_t speedup = baseline * RESULT_SCALE / prefetch;
  printf("  speedup: %llu.%0" STR(RESULT_PERCISION) "llux\n", speedup / RESULT_SCALE, speedup % RESULT_SCALE);
  return 0;
}
