#include "common.h"
#include "memory.h"
#include "fs.h"

#define INIT_FILE "/bin/hello"

void ramdisk_read(void *, off_t, size_t);
size_t fs_filesz(int fd);

#define STACK_SIZE (128 * PGSIZE)

uintptr_t loader() {
  Log("Loading user program %s...", INIT_FILE);

  int fd = fs_open(INIT_FILE, 0, 0);
  ssize_t len = fs_read(fd, (void *)0x4000000, fs_filesz(fd));
  fs_close(fd);
  Log("Read %d bytes", len);

  return 0x4000000;
}
