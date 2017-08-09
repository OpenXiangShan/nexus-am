#include "common.h"
#include "syscall.h"

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

_RegSet* do_syscall(_RegSet *r) {
  int ret = 0;
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  Log("syscall = %d", a[0]);

  switch (a[0]) {
    case SYS_none: break;
    case SYS_exit: sys_exit(a[1]); break;
    case SYS_read: ret = sys_read(a[1], (void *)a[2], a[3]); break;
    case SYS_write: ret = sys_write(a[1], (const void *)a[2], a[3]); break;
    case SYS_open: ret = sys_open((const char *)a[1], a[2], a[3]); break;
    case SYS_lseek: ret = sys_lseek(a[1], a[2], a[3]); break;
    case SYS_close: ret = sys_close(a[1]); break;
    case SYS_brk: ret = sys_brk(a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  SYSCALL_ARG1(r) = ret;
  return NULL;
}
