#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <klib.h>

static int pmem_fd;
static ucontext_t uc_example;

static void init_platform() {
  pmem_fd = shm_open("/native-pmem", O_RDWR | O_CREAT, 0700);
  assert(pmem_fd != -1);
  size_t size = 128 * 1024 * 1024; // 128MB
  ftruncate(pmem_fd, size);

  uintptr_t start = 0x100000;
  uintptr_t end = size;
  start = (uintptr_t)mmap((void *)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, pmem_fd, start);
  assert(start != (uintptr_t)-1);

  _heap.start = (void *)(start + 4096);  // this is to skip the trap entry
  _heap.end = (void *)end;

  getcontext(&uc_example);
}

class _Init {
  public : _Init() {
    init_platform();
  }
};

static _Init _init;

extern "C" {

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

void get_example_uc(_RegSet *r) {
  memcpy(&r->uc, &uc_example, sizeof(uc_example));
}

}
