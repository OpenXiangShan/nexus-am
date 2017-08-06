#include "common.h"
#include "memory.h"
#include "fs.h"

void ramdisk_read(void *, off_t, size_t);

#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
  Log("Loading user program %s...", filename);

  int fd = fs_open(filename, 0, 0);

  size_t size = fs_filesz(fd);
  size_t i;
  for (i = 0; i < size; i += PGSIZE) {
    void *pa = new_page();
    _map(as, DEFAULT_ENTRY + i, pa);
    fs_read(fd, pa, PGSIZE);
  }

  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
