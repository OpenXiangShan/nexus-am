#include "common.h"
#include <sys/syscall.h>

bool mm_brk(uint32_t new_brk);
int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, int count);
ssize_t fs_write(int fd, const void *buf, int count);
int fs_close(int fd);
off_t fs_lseek(int fd, off_t offset, int whence);

void sys_exit(int status) {
  Log("Program exit with status = %d", status);
  _halt(status);
}

int sys_open(const char *path, int flags, int mode) {
  return fs_open(path, flags, mode);
}

ssize_t sys_read(int fd, void *buf, int count) {
  return fs_read(fd, buf, count);
}

ssize_t sys_write(int fd, const void *buf, int count) {
  return fs_write(fd, buf, count);
}

off_t sys_lseek(int fd, off_t offset, int whence) {
  return fs_lseek(fd, offset, whence);
}

int sys_close(int fd) {
  return fs_close(fd);
}

int sys_brk(uintptr_t brk) {
  return mm_brk(brk);
}

void do_syscall(_RegSet *r) {
  int ret = 0;
  switch (r->eax) {
    case SYS_exit: sys_exit(r->ebx); break;
    case SYS_read: ret = sys_read(r->ebx, (void *)r->ecx, r->edx); break;
    case SYS_write: ret = sys_write(r->ebx, (const void *)r->ecx, r->edx); break;
    case SYS_open: ret = sys_open((const char *)r->ebx, r->ecx, r->edx); break;
    case SYS_lseek: ret = sys_lseek(r->ebx, r->ecx, r->edx); break;
    case SYS_close: ret = sys_close(r->ebx); break;
    case SYS_brk: ret = sys_brk(r->ebx); break;
    default: panic("Unhandled syscall ID = %d, eip = 0x%08x", r->eax, r->eip);
  }

  r->eax = ret;
}
