#include "common.h"
#include <sys/syscall.h>

uint32_t do_write(int fd, void *buf, int count) {
  int i;
  for (i = 0; i < count; i ++) {
    _putc(((uint8_t *)buf)[i]);
  }
  return count;
}

void do_exit(int status) {
  Log("Program exit with status = %d", status);
  _halt(status);
}

void do_syscall(_RegSet *r) {
  int ret = 0;
  switch (r->eax) {
    case SYS_write: ret = do_write(r->ebx, (void *)r->ecx, r->edx); break;
    case SYS_exit: do_exit(r->ebx); break;
    default: panic("Unhandled syscall ID = %d, eip = 0x%08x", r->eax, r->eip);
  }

  r->eax = ret;
}
