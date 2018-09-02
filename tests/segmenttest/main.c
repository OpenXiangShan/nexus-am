#include <am.h>
#include <amdev.h>
#include <klib.h>

_Context testcase_regs;

int ntraps = 0;
_Context* handler(_Event ev, _Context *ctx) {
  switch (ev.event) {
    case _EVENT_IRQ_TIMER:
      printf(".");
      break;
    case _EVENT_IRQ_IODEV: 
      while (1) {
        int key = read_key();
        if (key == _KEY_NONE) break;
        if (key & 0x8000) {
          printf("[D:%d]", key ^ 0x8000);
        } else {
          printf("[U:%d]/[TRAPS:%d]", key, ntraps);
          ntraps = 0;
        }
      }
      break;
    case _EVENT_YIELD:
      ntraps++;
      break;
  }
  return &testcase_regs;
}

void load_testcase() {
  extern char __fsimg_start;
  extern char __fsimg_end;
}

int main(){
  _ioe_init();
  _cte_init(handler);

  assert(!_intr_read());
  _intr_write(1);

  load_testcase();

  _yield();
  while(1) { }
  return 0;
}
