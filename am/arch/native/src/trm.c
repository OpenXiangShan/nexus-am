#include <am.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <klib.h>

void _trm_init() {
}

void _putc(char ch) {
  putchar(ch);
}

void _halt(int code) {
  printf("Exit (%d)\n", code);
  _exit(code);
}

_Area _heap;

static int pmem_fd;

void __attribute__ ((constructor)) init_platform() {
  pmem_fd = shm_open("/native-pmem", O_RDWR | O_CREAT, 0700);
  assert(pmem_fd != -1);
  size_t size = 128 * 1024 * 1024;
  ftruncate(pmem_fd, size);

  void *start = (void *)0x100000;
  void *end = (void *)size;
  start = mmap(start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, pmem_fd, (uintptr_t)start);
  assert(start != (void *)-1);

  _heap.start = start + 4096;  // this is to skip the trap entry
  _heap.end = end;
}

void shm_mmap(void *va, void *pa, int prot) {
  void *ret = mmap(va, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, pmem_fd, (uintptr_t)pa);
  assert(ret != (void *)-1);
//  printf("%p -> %p\n", va, pa);
}

void shm_munmap(void *va) {
  int ret = munmap(va, 4096);
  assert(ret == 0);
//  printf("%p -> 0\n", va);
}
