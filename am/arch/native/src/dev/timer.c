#include <am.h>
#include <amdev.h>
#include <sys/time.h>
#include <unistd.h>

static struct timeval boot_time;


size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  struct timeval now;
  gettimeofday(&now, NULL);
  long seconds = now.tv_sec - boot_time.tv_sec;
  long useconds = now.tv_usec - boot_time.tv_usec;
  return seconds * 1000 + (useconds + 500) / 1000;
}

void timer_init() {
  gettimeofday(&boot_time, NULL);
}

