#include <am.h>
#include <amdev.h>
#include <klib.h>

int ntraps = 0;
_RegSet* handler(_Event ev, _RegSet *regs) {
  for(const char *p = "intr:"; *p; p++) _putc(*p);
  _putc(_intr_read() ? '1' : '0');
  _putc('\n');

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
      // assert(_intr_read() == 0);
      ntraps++;
      break;
  }
  return regs;
}

#define _STR(x) _VAL(x)
#define _VAL(x) #x

#define MFC0(dst, src, sel) \
asm volatile("nop; nop; mfc0 %0, $"_STR(src)", %1; nop; nop\n\t":"=r"(dst):"i"(sel))

void read_write_cp0() {
  int reg;
#define CP0_OUT(src, sel) \
  reg = 0x12345678; \
  MFC0(reg, src, sel); \
  printf("CP0:{" #src ", " #sel "}=%x\n", reg);
  CP0_OUT(8, 0) // badvaddr
  CP0_OUT(9, 0) // count0
  CP0_OUT(9, 1) // count1
  CP0_OUT(11, 0) // compare
  CP0_OUT(12, 0) // status
  CP0_OUT(13, 0) // cause
  CP0_OUT(14, 0) // epc
}

int main(){
  _ioe_init();
  _asye_init(handler);
  read_write_cp0();
  assert(!_intr_read());
  _intr_write(1);
  //_make(_heap, void*)main, 0);
  printf("intr:%x\n", _intr_read());
  _yield();
  while (1) {
	for(int i = 0; i < 10; i++)
	  printf("-");
	printf(":yield, intr:%x\n", _intr_read());
	while(1); // no yield
  }
  return 0;
}
