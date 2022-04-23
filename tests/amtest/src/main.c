#include <amtest.h>

void (*entry)() = NULL; // mp entry

static const char *tests[256] = {
  ['h'] = "hello",
  ['H'] = "display this help message",
  ['i'] = "interrupt/yield test",
  ['d'] = "scan devices",
  ['m'] = "multiprocessor test",
  ['t'] = "real-time clock test",
  ['k'] = "readkey test",
  ['v'] = "display test",
  ['a'] = "audio test",
  ['p'] = "x86 virtual memory test",
};

int main(const char *args) {
  char arg = 's';
  switch (arg) {
    CASE('h', hello);
    CASE('i', hello_intr, IOE, CTE(simple_trap), REEH(simple_trap), RCEH(simple_trap));
    CASE('e', external_intr, IOE, CTE(external_trap), REEH(external_trap), RTEH(external_trap));
    CASE('d', devscan, IOE);
    CASE('m', finalize, PRE_MPE(args[1]), MPE(mp_print));
    CASE('t', rtc_test, IOE);
    CASE('k', keyboard_test, IOE);
    CASE('v', video_test, IOE);
    CASE('a', audio_test, IOE);
    CASE('p', vm_test, CTE(vm_handler), VME(simple_pgalloc, simple_pgfree));
    CASE('c', pmp_test, CTE(simple_trap));
    CASE('s', sv39_test, IOE, CTE(simple_trap));
    case 'H':
    default:
      printf("Usage: make run mainargs=*\n");
      for (int ch = 0; ch < 256; ch++) {
        if (tests[ch]) {
          printf("  %c: %s\n", ch, tests[ch]);
        }
      }
  }
  return 1;
}
