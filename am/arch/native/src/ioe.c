#include <am.h>
#include <sys/time.h>
#include <unistd.h>

static struct timeval boot_time;

ulong _uptime() {
  struct timeval now;
  gettimeofday(&now, NULL);
  long seconds = now.tv_sec - boot_time.tv_sec;
  long useconds = now.tv_usec - boot_time.tv_usec;
  return seconds * 1000 + (useconds + 500) / 1000;
}

#if defined(__i386__) || defined(__x86_64__)
static inline long long rdtsc() {
  u32 lo, hi;
  asm volatile ("rdtscp": "=a"(lo), "=d"(hi) : : "%ecx");
  return ((long long)hi << 32) | lo;
}
static long long init_tsc;
static void cycles_init() {
  init_tsc = rdtsc();
}
ulong _cycles() {
  return (rdtsc() - init_tsc) >> 10;
}
#else
static void cycles_init() {}
ulong _cycles() {
  return 0;
}
#endif

void gui_init();

void _ioe_init() {
  gui_init();
  gettimeofday(&boot_time, NULL);
  cycles_init();
}

