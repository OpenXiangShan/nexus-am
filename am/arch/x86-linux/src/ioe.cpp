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

ulong _cycles() {
  return 0;
}

void gui_init();

void _ioe_init() {
  gui_init();
  gettimeofday(&boot_time, NULL);
}

