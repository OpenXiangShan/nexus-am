#include <am.h>
#include <amdev.h>
#include <amtrace.h>
#include <klib.h>

_Context *uctx;
int ntraps = 0;

_Context* handler(_Event ev, _Context *ctx) {
  switch (ev.event) {
    case _EVENT_YIELD:
      break;
    case _EVENT_IRQ_TIMER:
    case _EVENT_IRQ_IODEV: 
      printf("==== interrupt (%s)  ===\n", ev.msg);
      break;
    case _EVENT_PAGEFAULT:
      printf("PF: %x %s%s%s\n",
        ev.ref,
        (ev.cause & _PROT_NONE) ? "[not present]" : "",
        (ev.cause & _PROT_READ) ? "[read fail]" : "",
        (ev.cause & _PROT_WRITE) ? "[write fail]" : "");
      break;
    case _EVENT_SYSCALL:
      printf("%d ", ctx->GPR1);
      break;
    default:
      assert(0);
  }

  if (uctx) {
    ctx = uctx;
    uctx = NULL;
  }
  return ctx;
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
  _cte_init(handler);
  _vme_init(alloc, free);

  _protect(&prot);

  uint8_t *ptr = (void*)((uintptr_t)(prot.area.start) +
     ((uintptr_t)(prot.area.end) - (uintptr_t)(prot.area.start)) / 2);

  int pgsz = 4096;

  void *up1 = alloc(pgsz);
  _map(&prot, ptr, up1, _PROT_WRITE);

  memcpy(up1, code, sizeof(code));
  printf("Code copied to %x execute\n", ptr);

  _Area k = { .start = kstk, .end = kstk + 4096 };
  _Area u = { .start = ptr + pgsz, .end = ptr + pgsz };

  uctx = _ucontext(&prot, u, k, ptr, 0);

  _intr_write(1);
  while (1) {
    _yield();
  }
  return 0;
}
