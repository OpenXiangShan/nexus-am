#include "common.h"
#include "memory.h"
#include "fs.h"

#define INIT_FILE "/bin/pal"

void ramdisk_read(void *, off_t, size_t);
size_t fs_filesz(int fd);

#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as) {
  Log("Loading user program %s...", INIT_FILE);

  int fd = fs_open(INIT_FILE, 0, 0);

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
