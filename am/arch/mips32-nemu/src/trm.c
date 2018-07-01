#include <am.h>
#include <npc.h>
#include <klib.h>

int main();

void _trm_init() {
  int ret = main();
  _halt(ret);
}

// -------------------- memory --------------------

extern unsigned int _heap_start, _heap_end; // symbols

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

// -------------------- debug console --------------------

volatile static char *csend = SERIAL_PORT + Tx;
volatile static char *crecv = SERIAL_PORT + Rx;

static char __attribute__((__noinline__)) get_stat(){
  volatile char *stat = SERIAL_PORT + STAT;
  return *stat;
}

void out_byte(char ch) {
  while((get_stat() >> 3) & 0x1);
  *csend = ch;
}

char in_byte() {
  if(!(get_stat() & 0x1)) return '\0';
  else return *crecv;
}

void _putc(char ch) {
  if(ch == '\n') {
    out_byte('\r');
  }
  out_byte(ch);
}


// -------------------- halting --------------------

void _halt(int code) {
  /*
  uint32_t cycles_low = get_perf_counter(12);
  uint32_t cycles_high = get_perf_counter(13);

  uint32_t icache_access_low = get_perf_counter(0);
  uint32_t icache_access_high = get_perf_counter(1);

  uint32_t icache_miss_low = get_perf_counter(2);
  uint32_t icache_miss_high = get_perf_counter(3);

  uint32_t load_low = get_perf_counter(4);
  uint32_t load_high = get_perf_counter(5);

  uint32_t store_low = get_perf_counter(6);
  uint32_t store_high = get_perf_counter(7);

  uint32_t load_cycles_low = get_perf_counter(14);
  uint32_t load_cycles_high = get_perf_counter(15);

  uint32_t store_cycles_low = get_perf_counter(16);
  uint32_t store_cycles_high = get_perf_counter(17);

  uint32_t mul_low = get_perf_counter(18);
  uint32_t mul_high = get_perf_counter(19);

  uint32_t div_low = get_perf_counter(20);
  uint32_t div_high = get_perf_counter(21);

  uint32_t dcache_r_access_low = get_perf_counter(22);
  uint32_t dcache_r_access_high = get_perf_counter(23);
 
  uint32_t dcache_w_access_low = get_perf_counter(24);
  uint32_t dcache_w_access_high = get_perf_counter(25);
  
  uint32_t dcache_r_miss_low = get_perf_counter(26);
  uint32_t dcache_r_miss_high = get_perf_counter(27);
  
  uint32_t dcache_w_miss_low = get_perf_counter(28);
  uint32_t dcache_w_miss_high = get_perf_counter(29);

  uint32_t dcache_r_miss_cycles_low = get_perf_counter(30);
  uint32_t dcache_r_miss_cycles_high = get_perf_counter(31);

  uint32_t time = _uptime();

  printk("cycles(low) = 0x%08x\t%u\n", cycles_low, cycles_low);
  printk("cycles(high) = 0x%08x\t%u\n", cycles_high, cycles_high);

  printk("icache access(low) = 0x%08x\t%u\n", icache_access_low, icache_access_low);
  printk("icache access(high) = 0x%08x\t%u\n", icache_access_high, icache_access_high);

  printk("icache miss(low) = 0x%08x\t%u\n", icache_miss_low, icache_miss_low);
  printk("icache miss(high) = 0x%08x\t%u\n", icache_miss_high, icache_miss_high);

  printk("load(low) = 0x%08x\t%u\n", load_low, load_low);
  printk("load(high) = 0x%08x\t%u\n", load_high, load_high);

  printk("store(low) = 0x%08x\t%u\n", store_low, store_low);
  printk("store(high) = 0x%08x\t%u\n", store_high, store_high);

  printk("load cycles(low) = 0x%08x\t%u\n", load_cycles_low, load_cycles_low);
  printk("load cycles(high) = 0x%08x\t%u\n", load_cycles_high, load_cycles_high);

  printk("store cycles(low) = 0x%08x\t%u\n", store_cycles_low, store_cycles_low);
  printk("store cycles(high) = 0x%08x\t%u\n", store_cycles_high, store_cycles_high);

  printk("mul(low) = 0x%08x\t%u\n", mul_low, mul_low);
  printk("mul(high) = 0x%08x\t%u\n", mul_high, mul_high);

  printk("div(low) = 0x%08x\t%u\n", div_low, div_low);
  printk("div(high) = 0x%08x\t%u\n", div_high, div_high);

  printk("dcache read access(low) = 0x%08x\t%u\n", dcache_r_access_low, dcache_r_access_low);
  printk("dcache read access(high) = 0x%08x\t%u\n", dcache_r_access_high, dcache_r_access_high);
  
  printk("dcache write access(low) = 0x%08x\t%u\n", dcache_w_access_low, dcache_w_access_low);
  printk("dcache write access(high) = 0x%08x\t%u\n", dcache_w_access_high, dcache_w_access_high);
  
  printk("dcache read miss(low) = 0x%08x\t%u\n", dcache_r_miss_low, dcache_r_miss_low);
  printk("dcache read miss(high) = 0x%08x\t%u\n", dcache_r_miss_high, dcache_r_miss_high);
  
  printk("dcache write miss(low) = 0x%08x\t%u\n", dcache_w_miss_low, dcache_w_miss_low);
  printk("dcache write miss(high) = 0x%08x\t%u\n", dcache_w_miss_high, dcache_w_miss_high);

  printk("dcache read miss cycles(low) = 0x%08x\t%u\n", dcache_r_miss_cycles_low, dcache_r_miss_cycles_low);
  printk("dcache read miss cycles(high) = 0x%08x\t%u\n", dcache_r_miss_cycles_high, dcache_r_miss_cycles_high);

  printk("time = 0x%08x\t%u\n", time, time);
  */

    /*
  const char *msg = (code == 0) ?
    "Exited (0).\n" :
    "Exited (error).\n";

  for (; *msg; msg ++) {
    _putc(*msg);
  }
  */

  GPIO_TRAP[0] = code;
  while(1);
}

