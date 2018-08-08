#include <am.h>
#include <amdev.h>
#include <klib.h>

_RegSet *ctx;
int ntraps = 0;

_RegSet* handler(_Event ev, _RegSet *regs) {
  switch (ev.event) {
    case _EVENT_YIELD:
      break;
    case _EVENT_IRQ_TIMER:
    case _EVENT_IRQ_IODEV: 
      printf(".");
      break;
    case _EVENT_PAGEFAULT:
      printf("PF: %x %s%s%s\n",
        ev.ref,
        (ev.cause & _PROT_NONE) ? "[not present]" : "",
        (ev.cause & _PROT_READ) ? "[read fail]" : "",
        (ev.cause & _PROT_WRITE) ? "[write fail]" : "");
      break;
    case _EVENT_SYSCALL:
      printf("%d ", regs->GPR1);
      break;
    default:
      assert(0);
  }

  if (ctx) {
    regs = ctx;
    ctx = NULL;
  }
  return regs;
}

static uintptr_t st;

static void *alloc(size_t size) {
  while (st % size != 0) st++;
  void *ret = (void *)st;
  st += size;
  return ret;
}

static void free(void *ptr) {
}

_Protect prot;


uint8_t code[] = {
  0x31, 0xc0, // xor %eax, %eax
  0x8d, 0xb6, 0x00, 0x00, 0x00, 0x00, // lea 0(%esi), %esi
  0x83, 0xc0, 0x01, // add $1, %eax
//  0x90, 0x90, // nop, nop
  0xcd, 0x80, // int $0x80
  0xeb, 0xf9, // jmp 8
};

uint8_t kstk[4096];

int main() {
  st = (uintptr_t)_heap.start;
  _ioe_init();
  _asye_init(handler);
  _pte_init(alloc, free);

  _protect(&prot);

  uint8_t *ptr = (void*)((uintptr_t)(prot.area.start) +
     ((uintptr_t)(prot.area.end) - (uintptr_t)(prot.area.start)) / 2);

  int pgsz = 4096;

  _map(&prot, ptr, alloc(pgsz), _PROT_WRITE);
  _map(&prot, ptr + pgsz, alloc(pgsz), _PROT_WRITE);

  _switch(&prot);

  memcpy(ptr, code, sizeof(code));

  printf("Code copied to %x execute\n", ptr);

  _Area k = { .start = kstk, .end = kstk + 4096 };
  _Area u = { .start = ptr + pgsz, .end = ptr + pgsz * 2 };

  ctx = _ucontext(&prot, u, k, ptr, NULL);
  _switch(&prot);

  _intr_write(1);
  while (1) {
    _yield();
  }
  return 0;
}
