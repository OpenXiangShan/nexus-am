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

_RegSet* do_syscall(uintptr_t *args) {
  int ret = 0;
  switch (args[0]) {
    case SYS_exit: sys_exit(args[1]); break;
    case SYS_read: ret = sys_read(args[1], (void *)args[2], args[3]); break;
    case SYS_write: ret = sys_write(args[1], (const void *)args[2], args[3]); break;
    case SYS_open: ret = sys_open((const char *)args[1], args[2], args[3]); break;
    case SYS_lseek: ret = sys_lseek(args[1], args[2], args[3]); break;
    case SYS_close: ret = sys_close(args[1]); break;
    case SYS_brk: ret = sys_brk(args[1]); break;
    default: panic("Unhandled syscall ID = %d", args[0]);
  }

  args[0] = ret;
  return NULL;
}
