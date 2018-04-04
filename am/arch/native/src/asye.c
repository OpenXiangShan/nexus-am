#include <am.h>
#include <klib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

static _RegSet* (*H)(_Event, _RegSet*) = NULL;

extern void asm_trap();
extern void ret_from_trap();

void irq_handle(ucontext_t *uc) {
  getcontext(uc);

  _RegSet *r = (void *)uc->uc_mcontext.gregs;
  uintptr_t *rax = (void *)uc + 1024;
  SYSCALL_ARG1(r) = *rax;

  _Event e;
  e.event = _EVENT_SYSCALL;
  _RegSet *ret = H(e, r);
  assert(ret == NULL);

  r->regs[REG_RIP] = (uintptr_t)ret_from_trap;
  r->regs[REG_RSP] = (uintptr_t)uc;
  *rax = SYSCALL_ARG1(r);

  setcontext(uc);
}

int pmem_fd;

int _asye_init(_RegSet*(*handler)(_Event, _RegSet*)) {
  pmem_fd = shm_open("/native-pmem", O_RDWR | O_CREAT, 0700);
  assert(pmem_fd != -1);
  size_t size = 128 * 1024 * 1024;
  ftruncate(fd, size);

  void *start = (void *)0x100000;
  void *end = (void *)size;
  start = mmap(start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE | MAP_FIXED, pmem_fd, (uintptr_t)start);
  assert(start != (void *)-1);

  *(uintptr_t *)start = (uintptr_t)asm_trap;

  _heap.start = start + 4096;
  _heap.end = end;

  H = handler;
  return 0;
}

_RegSet *_make(_Area stack, void (*entry)(void *), void *arg) {
  return NULL;
}

void _yield() {
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
