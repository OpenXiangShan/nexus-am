#include <amtest.h>

static _Context uctx;
static _AddressSpace prot;
static uintptr_t st = 0;
static _Context *current = NULL;

void *simple_pgalloc(size_t size) {
  if (st == 0) { st = (uintptr_t)_heap.start; }
  while (st % size != 0) st++;
  void *ret = (void *)st;
  st += size;
  return ret;
}

void simple_pgfree(void *ptr) {
}

_Context* vm_handler(_Event ev, _Context *ctx) {
  if (current) {
    *current = *ctx;
  } else {
    current = &uctx;
  }
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
        (ev.cause & _PROT_NONE)  ? "[not present]" : "",
        (ev.cause & _PROT_READ)  ? "[read fail]"   : "",
        (ev.cause & _PROT_WRITE) ? "[write fail]"  : "");
      break;
    case _EVENT_SYSCALL:
      printf("%d ", ctx->GPRx);
      break;
    default:
      assert(0);
  }
  return current;
}

uint8_t code[] = {
  0x31, 0xc0,             // xor %eax, %eax
  0x8d, 0xb6,             // lea 0(%esi), %esi
  0x00, 0x00, 0x00, 0x00,
  0x83, 0xc0, 0x01,       // add $1, %eax
  0xcd, 0x80,             // int $0x80
  0xeb, 0xf9,             // jmp 8
};

void vm_test() {
  if (strncmp(__ISA__, "x86", 3) != 0) {
    printf("VM test: only runs on x86.\n");
    return;
  }
  _protect(&prot);

  uint8_t *ptr = (void*)((uintptr_t)(prot.area.start) +
     ((uintptr_t)(prot.area.end) - (uintptr_t)(prot.area.start)) / 2);

  int pgsz = 4096;

  void *up1 = simple_pgalloc(pgsz);
  _map(&prot, ptr, up1, _PROT_WRITE);

  memcpy(up1, code, sizeof(code));
  printf("Code copied to %p (physical %p) execute\n", ptr, up1);

  static uint8_t kstk[4096];
  _ucontext(&uctx, &prot, (_Area) { kstk, kstk + 4096 } , ptr);

  _intr_write(1);
  while (1) ;
}
