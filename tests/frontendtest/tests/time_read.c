#include <klib.h>
#include <amdev.h>
#include <stdint.h>

uint32_t uptime() {
  _DEV_TIMER_UPTIME_t uptime;
  _io_read(_DEV_TIMER, _DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  return uptime.lo;
}

int main() {
  uint32_t start = uptime();
  printf("start: %d\n", start);
  int sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += i;
    // printf("i: %d\n", i);

  }
  uint32_t end = uptime();
  printf("sum: %d\n", sum);
  printf("time: %d\n", end - start);
  return 0;
}