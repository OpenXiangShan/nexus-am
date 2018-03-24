#include <am.h>
#include <amdev.h>
#include <klib.h>

int ntraps = 0;
_RegSet* handler(_Event ev, _RegSet *regs) {
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
      assert(_get_intr() == 0);
      ntraps++;
      break;
  }
  return regs;
}

int main(){
  _ioe_init();
  _asye_init(handler);
  assert(!_get_intr());
  _set_intr(1);
  //_make(_heap, void*)main, 0);
  while (1) {
    _yield();
  }
  return 0;
}
