#include <amtest.h>

void (*entry)() = NULL; // mp entry

static const char *tests[256] = {
  ['h'] = "hello",
  ['H'] = "hello with interrupt enabled",
  ['d'] = "device scanning",
  ['m'] = "multiprocessor",
  ['t'] = "real-time clock",
};

int main(const char *args) {
  switch (args[0]) {
    CASE('h', hello)
    CASE('H', hello_intr, IOE, CTE(NULL))
    CASE('d', devscan, IOE)
    CASE('m', mp_print, MPE)
    CASE('t', show_time, IOE)
    default:
      printf("Usage: make run mainargs=*\n");
      for (int ch = 0; ch < 256; ch++) {
        if (tests[ch]) {
          printf("  %c: %s\n", ch, tests[ch]);
        }
      }
  }
  return 0;
}
