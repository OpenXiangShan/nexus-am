#include <amtest.h>

void (*entry)() = NULL; // mp entry

static const char *tests[256] = {
  ['h'] = "hello",
  ['H'] = "display this help message",
  ['i'] = "interrupt/yield test",
  ['e'] = "external interrupt (PLIC) test",
  ['d'] = "scan devices",
  ['m'] = "multiprocessor test",
  ['t'] = "real-time clock test",
  ['k'] = "readkey test",
  ['v'] = "display test",
  ['a'] = "audio test",
  ['p'] = "x86 virtual memory test",
  ['c'] = "risc-v physical memory protection test",
  ['s'] = "risc-v virtual memory test",
  ['r'] = "risc-v RTC tick test",
  ['z'] = "risc-v soft intr test",
};

int main(const char *args) {
  switch (args[0]) {
    CASE('x', dma_test);
    CASE('h', hello);
    CASE('i', hello_intr, IOE, CTE(simple_trap), REEH(simple_trap), RCEH(simple_trap), RTEH(simple_trap));
    CASE('z', soft_intr, IOE, CTE(soft_trap), RSEH(soft_trap), REEH(soft_trap), RCEH(soft_trap), RTEH(soft_trap));
    CASE('e', external_intr, IOE, NOTIMEINT(), CTE(external_trap), REEH(external_trap), RTEH(external_trap));
    CASE('u', test_BEU, IOE, NOTIMEINT(), CTE(handle_external_trap), REEH(handle_external_trap), RTEH(handle_external_trap));
    CASE('d', devscan, IOE);
    CASE('m', finalize, PRE_MPE(args[1]), MPE(mp_print));
    CASE('t', rtc_test, IOE);
    CASE('k', keyboard_test, IOE);
    CASE('v', video_test, IOE);
    CASE('a', audio_test, IOE);
    CASE('p', vm_test, CTE(vm_handler), VME(simple_pgalloc, simple_pgfree));
    CASE('c', pmp_test, CTE(simple_trap));
    CASE('s', sv39_test, IOE, CTE(simple_trap));
    CASE('f', sv39_hp_atom_test, IOE, CTE(simple_trap));
    CASE('b', cache_test);
    CASE('r', rtc_accuracy_test);
    case 'H':
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
