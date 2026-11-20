#include <am.h>
#include <klib.h>
#include <stdint.h>

/* Testbench command interface; all tested data uses the real SoC DMA AXI port. */
#define CONTROL 0x40080000UL
#define REG(off) (*(volatile uint64_t *)(CONTROL + (off)))
#define LINE 64UL
#define TEST_BASE 0x81000000UL
#define CMD 0x08
#define ADDR 0x10
#define MASK 0x18
#define CONFIG 0x20
#define ID 0x28
#define STATUS 0x30
#define REPEATS 0x38
#define STRIDE 0x40
#define COMPLETED 0x48
#define WATCH_BASE 0x80
#define CLEAR 0x88
#define WATCH_MASK 0x90
#define WDATA 0x100
#define RDATA 0x200
#define COUNT(n) REG(0x400 + 8*(n))
enum { GET, PROBE, PROBE_DATA, DMA_GRANT, REPLAY, DMA_HIT,
       SBUFFER_GRANT, CONTENTION, RELEASE_DATA, REFILL, DDR_AR, DDR_AW };

static volatile uint64_t cold_image[8] __attribute__((aligned(4096))) = {
  0x1029384756abcdefUL, 0x1122334455667788UL,
  0x8877665544332211UL, 0xfedcba9876543210UL,
  0xa5a55a5af0f00f0fUL, 0x0123456789abcdefUL,
  0xdeadbeef12345678UL, 0x76543210cafebabeUL
};
static const uint64_t cold_expected[8] = {
  0x1029384756abcdefUL, 0x1122334455667788UL,
  0x8877665544332211UL, 0xfedcba9876543210UL,
  0xa5a55a5af0f00f0fUL, 0x0123456789abcdefUL,
  0xdeadbeef12345678UL, 0x76543210cafebabeUL
};
static uint64_t tx[4];
static uint64_t rx[64];
static unsigned test_number;

static inline void fence(void) { asm volatile("fence iorw, iorw" ::: "memory"); }
static void fail(const char *what, uint64_t at, uint64_t got, uint64_t expected) {
  printf("[DMA FAIL] case=%d %s at=%lx got=%lx expected=%lx\n",
         test_number, what, at, got, expected);
  _halt(1);
}
static void check(int cond, const char *what, uint64_t got) {
  if (!cond) fail(what, 0, got, 1);
}
static void equal(uint64_t at, uint64_t got, uint64_t expected) {
  if (got != expected) fail("data", at, got, expected);
}
static void watch(uint64_t base, uint64_t bytes) {
  REG(WATCH_BASE) = base;
  REG(WATCH_MASK) = ~(bytes-1);
  REG(CLEAR) = 1;
  fence();
}
static void begin(int write, uint64_t addr, unsigned beats, uint32_t mask,
                  unsigned repeats, uint64_t stride) {
  REG(ADDR) = addr;
  REG(MASK) = mask;
  REG(CONFIG) = 5 | ((uint64_t)(beats-1) << 8) | (1UL << 16);
  REG(ID) = (0xa35 + test_number) & 0xfff;
  REG(REPEATS) = repeats;
  REG(STRIDE) = stride;
  for (unsigned i=0;i<4;i++) REG(WDATA+8*i) = tx[i];
  fence();
  REG(CMD) = write;
  fence();
}
static void wait_done(unsigned expected) {
  uint64_t s, start, now;
  asm volatile("rdcycle %0" : "=r"(start));
  do {
    s = REG(STATUS);
    asm volatile("rdcycle %0" : "=r"(now));
    if (now-start > 2000000UL) fail("AXI command made no completion",REG(COMPLETED),s,2);
  } while (!(s & 2));
  if (s >> 8) fail("AXI response/protocol", 0, s, 2);
  equal(COMPLETED, REG(COMPLETED), expected);
  fence();
}
static void read_dma(uint64_t addr, unsigned beats) {
  begin(0,addr,beats,0xffffffffU,1,0);
  wait_done(1);
  for(unsigned i=0;i<4*beats;i++) rx[i]=REG(RDATA+8*i);
}
static void write_dma(uint64_t addr, unsigned beats, uint32_t mask) {
  begin(1,addr,beats,mask,1,0);
  wait_done(1);
}
static uint64_t pattern(uint64_t addr, unsigned word, uint64_t salt) {
  return 0x9e3779b97f4a7c15UL ^ (addr*0x100000001b3UL) ^
    ((uint64_t)word*0x0102030405060708UL) ^ salt;
}
static void set_tx(uint64_t addr, uint64_t salt) {
  for(unsigned i=0;i<4;i++) tx[i]=pattern(addr,i,salt);
}
static void pass(const char *name) {
  printf("[DMA PASS] %d %s\n",test_number,name);
  printf("  coverage get=%d probeData=%d dma=%d replay=%d hit=%d sb=%d contention=%d release=%d DDRr=%d DDRw=%d\n",
    (int)COUNT(GET),(int)COUNT(PROBE_DATA),(int)COUNT(DMA_GRANT),
    (int)COUNT(REPLAY),(int)COUNT(DMA_HIT),(int)COUNT(SBUFFER_GRANT),
    (int)COUNT(CONTENTION),(int)COUNT(RELEASE_DATA),(int)COUNT(DDR_AR),(int)COUNT(DDR_AW));
}

static void test_cold_read(void) {
  test_number=1;
  uint64_t a=(uintptr_t)cold_image;
  watch(a,LINE);
  read_dma(a,2);
  for(unsigned i=0;i<8;i++) equal(a+8*i,rx[i],cold_expected[i]);
  check(COUNT(GET)>0,"Get coverage",COUNT(GET));
  check(COUNT(DDR_AR)>0,"cold L2 miss/DDR coverage",COUNT(DDR_AR));
  pass("cold AXI read through L2/DDR");
}
static void test_dirty_read(void) {
  test_number=2;
  uint64_t a=TEST_BASE;
  watch(a,LINE);
  volatile uint64_t *p=(void *)a;
  for(unsigned i=0;i<8;i++) p[i]=pattern(a,i,0x22);
  fence();
  read_dma(a,2);
  for(unsigned i=0;i<8;i++) equal(a+8*i,rx[i],pattern(a,i,0x22));
  check(COUNT(PROBE_DATA)>0,"dirty probe data coverage",COUNT(PROBE_DATA));
  equal(DDR_AW,COUNT(DDR_AW),0); /* DDR never received the new dirty value. */
  pass("CPU dirty DCache -> DMA read with real probe");
}
static void test_write_cpu(void) {
  test_number=3;
  uint64_t a=TEST_BASE+0x10000;
  volatile uint64_t *p=(void *)a;
  for(unsigned i=0;i<8;i++) p[i]=pattern(a,i,0x33);
  fence();
  watch(a,LINE);
  set_tx(a,0x3344);
  write_dma(a,2,0xffffffffU);
  for(unsigned i=0;i<8;i++) equal(a+8*i,p[i],tx[i%4]);
  check(COUNT(DMA_HIT)>=2,"DCache successful write coverage",COUNT(DMA_HIT));
  pass("DMA write -> CPU load");
}
static void test_write_read(void) {
  test_number=4;
  uint64_t a=TEST_BASE+0x20000;
  set_tx(a,0x4455);
  watch(a,LINE);
  write_dma(a,2,0xffffffffU);
  read_dma(a,2);
  for(unsigned i=0;i<8;i++) equal(a+8*i,rx[i],tx[i%4]);
  volatile uint64_t *p=(void *)a;
  for(unsigned i=0;i<8;i++) equal(a+8*i,p[i],tx[i%4]);
  check(COUNT(GET)>0,"read-after-write Get coverage",COUNT(GET));
  pass("DMA write -> DMA read, independently checked by CPU");
}
static void test_partial(void) {
  test_number=5;
  uint64_t a=TEST_BASE+0x30000;
  volatile uint8_t *p=(void *)a;
  uint8_t expected[64];
  for(unsigned i=0;i<64;i++) { expected[i]=(i*7+3)&255; p[i]=expected[i]; }
  fence();
  watch(a,LINE);
  uint32_t masks[3]={0xa55a8001U,0x00ffff00U,0};
  for(unsigned t=0;t<3;t++) {
    set_tx(a,0x5500+t);
    unsigned half=t&1;
    write_dma(a+half*32,1,masks[t]);
    for(unsigned i=0;i<32;i++) if((masks[t]>>i)&1)
      expected[half*32+i]=tx[i/8]>>(8*(i%8));
    for(unsigned i=0;i<64;i++) equal(a+i,p[i],expected[i]);
  }
  read_dma(a,2);
  for(unsigned i=0;i<64;i++) equal(a+i,(rx[i/8]>>(8*(i%8)))&255,expected[i]);
  equal(DMA_GRANT,COUNT(DMA_GRANT),2); /* Zero WSTRB issued no DCache request. */
  pass("sparse/contiguous/zero WSTRB, both cache-line halves");
}
static void test_cold_write(void) {
  test_number=6;
  const uint64_t a=0x82000000UL;
  const unsigned n=128;
  watch(a,0x1000000);
  set_tx(a,0x6677);
  begin(1,a,2,0xffffffffU,n,0x10000);
  /* Real same-set CPU stores compete for miss entries and replacement ways. */
  for(unsigned round=0;round<4;round++)
    for(unsigned line=0;line<128;line++)
      *(volatile uint64_t *)(0x90000000UL+line*0x10000UL)=pattern(line,round,0x6666);
  fence();
  wait_done(n);
  check(COUNT(REPLAY)>0,"real cold-write replay coverage",COUNT(REPLAY));
  check(COUNT(DMA_GRANT)>2*n,"real DMA retry coverage",COUNT(DMA_GRANT));
  equal(DMA_HIT,COUNT(DMA_HIT),2*n);
  for(unsigned line=0;line<n;line++) {
    volatile uint64_t *p=(void *)(a+line*0x10000UL);
    for(unsigned i=0;i<8;i++) equal((uintptr_t)&p[i],p[i],tx[i%4]);
  }
  pass("cold DCache writes under CPU pressure, real replay/retry/success");
}
static void test_concurrent(void) {
  test_number=7;
  const uint64_t dma=TEST_BASE+0x300000, cpu=TEST_BASE+0x600000;
  const unsigned n=128;
  watch(TEST_BASE,0x1000000);
  set_tx(dma,0x7788);
  begin(1,dma,1,0xffffffffU,n,LINE);
  for(unsigned round=0;round<4;round++) {
    for(unsigned line=0;line<n;line++) {
      volatile uint64_t *p=(void *)(cpu+line*LINE);
      for(unsigned i=0;i<8;i++) p[i]=pattern(cpu+line*LINE,i,round);
    }
  }
  fence();
  wait_done(n);
  check(COUNT(CONTENTION)>0,"simultaneous DMA/Sbuffer arbitration",COUNT(CONTENTION));
  check(COUNT(SBUFFER_GRANT)>0,"Sbuffer service coverage",COUNT(SBUFFER_GRANT));
  check(COUNT(DMA_GRANT)>=n,"DMA service coverage",COUNT(DMA_GRANT));
  for(unsigned line=0;line<n;line++) {
    volatile uint64_t *d=(void *)(dma+line*LINE), *c=(void *)(cpu+line*LINE);
    for(unsigned i=0;i<4;i++) equal(dma+line*LINE+8*i,d[i],tx[i]);
    for(unsigned i=0;i<8;i++) equal(cpu+line*LINE+8*i,c[i],pattern(cpu+line*LINE,i,3));
  }
  pass("CPU Sbuffer + DMA simultaneous writes, both datasets checked");
}
static void test_eviction(void) {
  test_number=8;
  const uint64_t a=TEST_BASE+0x400000;
  set_tx(a,0x8899);
  write_dma(a,2,0xffffffffU);
  watch(a,LINE);
  volatile uint64_t sum=0;
  /* Same set/bank, many distinct tags: evict from both L1D and the real L2. */
  for(unsigned round=0;round<3;round++)
    for(unsigned i=0;i<64;i++)
      sum += *(volatile uint64_t *)(0x84000000UL+i*0x100000UL);
  fence();
  check(COUNT(RELEASE_DATA)>0,"dirty DCache replacement coverage",COUNT(RELEASE_DATA));
  check(COUNT(DDR_AW)>0,"target L2 writeback to DDR coverage",COUNT(DDR_AW));
  read_dma(a,2);
  for(unsigned i=0;i<8;i++) equal(a+8*i,rx[i],tx[i%4]);
  check(COUNT(DDR_AR)>0,"target DDR reread coverage",COUNT(DDR_AR));
  volatile uint64_t *p=(void *)a;
  for(unsigned i=0;i<8;i++) equal(a+8*i,p[i],tx[i%4]);
  pass("actual L1/L2 eviction, DDR writeback and refill");
  (void)sum;
}
int main(const char *args) {
  (void)args;
  equal(CONTROL,REG(0),0x444d415445535431UL);
  printf("DMA AXI system test: full SoC, software self-check\n");
  test_cold_read();
  test_dirty_read();
  test_write_cpu();
  test_write_read();
  test_partial();
  test_cold_write();
  test_concurrent();
  test_eviction();
  printf("DMA SYSTEM TEST: ALL 8 CASES PASSED\n");
  return 0;
}
