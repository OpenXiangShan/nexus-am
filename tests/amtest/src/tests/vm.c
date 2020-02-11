#include <amtest.h>

static _Context *uctx;
static _AddressSpace prot;
static uintptr_t st = 0;
static int first_trap = 1;

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
  switch (ev.event) {
    case _EVENT_YIELD:
      break;
    case _EVENT_IRQ_TIMER:
    case _EVENT_IRQ_IODEV:
      printf("==== interrupt (%s)  ====\n", ev.msg);
      break;
    case _EVENT_PAGEFAULT:
      printf("PF: %x %s%s%s\n",
        ev.ref,
        (ev.cause & _PROT_NONE)  ? "[not present]" : "",
        (ev.cause & _PROT_READ)  ? "[read fail]"   : "",
        (ev.cause & _PROT_WRITE) ? "[write fail]"  : "");
      break;
    case _EVENT_SYSCALL:
      _intr_write(1);
      printf("%d ", ctx->GPRx);
      break;
    default:
      assert(0);
  }
  if (first_trap) {
    first_trap = 0;
    return uctx;
  } else {
    return ctx;
  }
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
  _protect(&prot);
  printf("Protected address space: [%p, %p)\n", prot.area.start, prot.area.end);

  uint8_t *ptr = (void*)((uintptr_t)(prot.area.start) +
     ((uintptr_t)(prot.area.end) - (uintptr_t)(prot.area.start)) / 2);

  void *pg = simple_pgalloc(prot.pgsize);
  memcpy(pg, code, sizeof(code));

  _map(&prot, ptr, pg, _PROT_WRITE | _PROT_READ | _PROT_EXEC);
  printf("Code copied to %p (physical %p) execute\n", ptr, pg);

  static uint8_t stack[4096];
  uctx = _ucontext(&prot, RANGE(stack, stack + sizeof(stack)), ptr);

  _intr_write(1);
  while (1) ;
}
