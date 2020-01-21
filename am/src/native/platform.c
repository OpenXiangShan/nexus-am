#include <sys/mman.h>
#include <sys/auxv.h>
#include <fcntl.h>
#include <elf.h>
#include <stdlib.h>
#include "platform.h"

#define PMEM_SIZE (128 * 1024 * 1024) // 128MB
static int pmem_fd = 0;
static char pmem_shm_file[] = "/native-pmem-XXXXXX";
static void *pmem = NULL;

#define PRIVATE_MEM_START (void *)0x100000
#define PRIVATE_MEM_SIZE 4096
void *__am_private_alloc(size_t n) {
  static void *p = PRIVATE_MEM_START + sizeof(uintptr_t);  // skip syscall entry
  void *ret = p;
  p += n;
  assert(p < PRIVATE_MEM_START + PRIVATE_MEM_SIZE);
  return ret;
}

static ucontext_t uc_example = {};

int main(const char *args);

static void init_platform() __attribute__((constructor));
static void init_platform() {
  // create shared memory object and set up mapping to simulate the physical memory
  mktemp(pmem_shm_file);
  pmem_fd = shm_open(pmem_shm_file, O_RDWR | O_CREAT | O_EXCL, 0700);
  assert(pmem_fd != -1);
  assert(0 == ftruncate(pmem_fd, PMEM_SIZE));

  pmem = mmap(NULL, PMEM_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, pmem_fd, 0);
  assert(_heap.start != (void *)-1);

  // create private memory to simulate per-cpu data
  void *ret = mmap(PRIVATE_MEM_START, PRIVATE_MEM_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  assert(ret != (void *)-1);

  // remap writable sections as MAP_SHARED
  Elf64_Phdr *phdr = (void *)getauxval(AT_PHDR);
  int phnum = (int)getauxval(AT_PHNUM);
  int i;
  int ret2;
  for (i = 0; i < phnum; i ++) {
    if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_W)) {
      // allocate temporary memory
      extern char end;
      void *vaddr = (void *)&end - phdr[i].p_memsz;
      uintptr_t pad = (uintptr_t)vaddr & 0xfff;
      void *vaddr_align = vaddr - pad;
      uintptr_t size = phdr[i].p_memsz + pad;
      void *temp_mem = malloc(size);
      assert(temp_mem != NULL);

      // save data and bss sections
      memcpy(temp_mem, vaddr_align, size);

      // save the addresses of library functions which will be used after munamp()
      // since calling the library functions requires accessing GOT, which will be unmapped
      void *(*volatile mmap_libc)(void *, size_t, int, int, int, off_t) = &mmap;
      void *(*volatile memcpy_libc)(void *, const void *, size_t) = &memcpy;

      // unmap the data and bss sections
      ret2 = munmap(vaddr_align, size);
      assert(ret2 == 0);

      // map the sections again with MAP_SHARED, which will be shared across fork()
      ret = mmap_libc(vaddr_align, size, PROT_READ | PROT_WRITE | PROT_EXEC,
          MAP_SHARED | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
      assert(ret == vaddr_align);

      // restore the data in the sections
      memcpy_libc(vaddr_align, temp_mem, size);

      free(temp_mem);
    }
  }

  // set up the AM heap
  _heap.start = pmem;
  _heap.end = pmem + PMEM_SIZE;

  getcontext(&uc_example);

  // block SIGVTALRM and SIGUSR1 to simulate disabling interrupt
  ret2 = sigaddset(&uc_example.uc_sigmask, SIGVTALRM);
  assert(ret2 == 0);
  ret2 = sigaddset(&uc_example.uc_sigmask, SIGUSR1);
  assert(ret2 == 0);

  const char *args = getenv("mainargs");
  exit(main(args ? args : "")); // call main here!
}

static void exit_platform() __attribute__((destructor));
static void exit_platform() {
  printf("%s\n", __func__);
  kill(0, SIGKILL);

  int ret = munmap(pmem, PMEM_SIZE);
  assert(ret == 0);
  close(pmem_fd);
  ret = shm_unlink(pmem_shm_file);
  assert(ret == 0);

  ret = munmap(PRIVATE_MEM_START, PRIVATE_MEM_SIZE);
  assert(ret == 0);
}

void __am_shm_mmap(void *va, void *pa, int prot) {
  void *ret = mmap(va, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, pmem_fd, (uintptr_t)(pa - pmem));
  assert(ret != (void *)-1);
}

void __am_shm_munmap(void *va) {
  int ret = munmap(va, 4096);
  assert(ret == 0);
}

void __am_get_example_uc(_Context *r) {
  memcpy(&r->uc, &uc_example, sizeof(uc_example));
}

// This dummy function will be called in trm.c.
// The purpose of this dummy function is to let linker add this file to the object
// file set. Without it, the constructor of @_init_platform will not be linked.
void __am_platform_dummy() {
}
