#include "../../x86-qemu.h"

static _DEV_TIMER_DATE_t boot_date;
static uint32_t freq_mhz = 2000;
uint64_t uptsc;

static inline uint64_t rdtsc() {
  uint32_t lo, hi;
  asm volatile ("rdtsc": "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

static int read_rtc(int reg) {
  outb(0x70, reg);
  int ret = inb(0x71);
  return (ret & 0xf) + (ret >> 4) * 10;
}

static void wait_sec() {
  while (1) {
    int volatile s1 = read_rtc(0);
    for (int volatile i = 0; i < 10000; i++) ;
    int volatile s2 = read_rtc(0);
    if (s1 != s2) {
      return;
    }
  }
}

static uint32_t estimate_freq() {
  int h, m, s, t1, t2;
  uint64_t tsc1, tsc2;
  wait_sec();
  tsc1 = rdtsc();
  h = read_rtc(4); m = read_rtc(2); s = read_rtc(0);
  t1 = h * 3600 + m * 60 + s;
  wait_sec();
  tsc2 = rdtsc();
  h = read_rtc(4); m = read_rtc(2); s = read_rtc(0);
  t2 = h * 3600 + m * 60 + s;
  if (t1 >= t2) return 0; // passed a day, unlikely to happen

  uint32_t freq = (tsc2 - tsc1) >> 20;
  freq /= (t2 - t1);
  return freq;
}

static void get_date(_DEV_TIMER_DATE_t *rtc) {
  int tmp;
  do {
    rtc->second = read_rtc(0);
    rtc->minute = read_rtc(2);
    rtc->hour   = read_rtc(4);
    rtc->day    = read_rtc(7);
    rtc->month  = read_rtc(8);
    rtc->year   = read_rtc(9) + 2000;
    tmp         = read_rtc(0);
  } while (tmp != rtc->second);
}

void timer_init() {
  freq_mhz = estimate_freq();
  get_date(&boot_date);
  uptsc = rdtsc();
}

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      uint64_t tsc = rdtsc() - uptsc;
      uint32_t mticks = (tsc >> 20);
      uint32_t ms = mticks * 1000 / freq_mhz;
      _DEV_TIMER_UPTIME_t *uptime = (_DEV_TIMER_UPTIME_t *)buf;
      uptime->hi = 0;
      uptime->lo = ms;
      return sizeof(_DEV_TIMER_UPTIME_t);
    }
    case _DEVREG_TIMER_DATE: {
      get_date((_DEV_TIMER_DATE_t *)buf);
      return sizeof(_DEV_TIMER_DATE_t);
    }
  }
  return 0;
}
