#include <sys/mman.h>
#include <sys/auxv.h>
#include <signal.h>
#include <fcntl.h>
#include <elf.h>
#include <stdlib.h>
#include <klib.h>
#include "platform.h"

#define PMEM_SIZE (128 * 1024 * 1024) // 128MB
#define PMEM_MAP_START (uintptr_t)0x100000
#define PMEM_MAP_END   (uintptr_t)PMEM_SIZE
#define PMEM_MAP_SIZE  (PMEM_MAP_END - PMEM_MAP_START)

static int pmem_fd = 0;
static char pmem_shm_file[] = "/native-pmem-XXXXXX";
static void *pmem = NULL;
static ucontext_t uc_example = {};
uintptr_t __am_rebase_offset = 0;

#define PRIVATE_MEM_START (void *)0x100000
#define PRIVATE_MEM_SIZE 4096
void *__am_private_alloc(size_t n) {
  static void *p = PRIVATE_MEM_START + sizeof(uintptr_t);  // skip syscall entry
  void *ret = p;
  p += n;
  assert(p < PRIVATE_MEM_START + PRIVATE_MEM_SIZE);
  return ret;
}

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

  // compute ASLR offset
  Elf64_Phdr *phdr = (void *)getauxval(AT_PHDR);
  int phnum = (int)getauxval(AT_PHNUM);
  extern char end;
  uintptr_t aslr_offset = 0;
  int i;
  int ret2;
  for (i = 0; i < phnum; i ++) {
    if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_W)) {
      void *end_in_elf = (void *)(phdr[i].p_vaddr + phdr[i].p_memsz);
      aslr_offset = (void *)&end - end_in_elf;
      assert((aslr_offset & 0xfff) == 0);

      // allocate temporary memory
      uintptr_t vaddr = phdr[i].p_vaddr + aslr_offset;
      uintptr_t pad = vaddr & 0xfff;
      void *vaddr_align = (void *)(vaddr - pad);
      uintptr_t size = phdr[i].p_memsz + pad;
      void *temp_mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      assert(temp_mem != (void *)-1);

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
      ret = mmap_libc(vaddr_align, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
      assert(ret == vaddr_align);

      // restore the data in the sections
      memcpy_libc(vaddr_align, temp_mem, size);

      // unmap the temporary memory
      ret2 = munmap(temp_mem, size);
      assert(ret2 == 0);
    }
  }
  assert(aslr_offset != 0);
  __am_rebase_offset = aslr_offset - PMEM_MAP_START;

  // set up the AM heap
  _heap.start = pmem;
  _heap.end = pmem + PMEM_SIZE;

  getcontext(&uc_example);

  // block SIGVTALRM to simulate disabling interrupt
  ret2 = sigaddset(&uc_example.uc_sigmask, SIGVTALRM);
  assert(ret2 == 0);

#if 0
  // relocation, now we should not write any global variables
  // before calling the rebase verison of main(), else the update
  // can not be catched by the relocation process
  for (i = 0; i < phnum; i ++) {
    if (phdr[i].p_type == PT_LOAD) {
      void *vaddr = (void *)(phdr[i].p_vaddr + aslr_offset);
      memcpy(REBASE_PTR(vaddr), vaddr, phdr[i].p_memsz);
    }
  }
  assert(*(int *)PMEM_MAP_START == 0x464c457f);

  // NOTE: the heap is not relocated, so do not use memory
  // returnedby malloc() in the code

  // relocate the vaule of data
  extern Elf64_Dyn _DYNAMIC[];
  Elf64_Dyn *pDyn;
  Elf64_Rela *rela_dyn = NULL;
  int nr_rela_dyn = 0;
  for (pDyn = _DYNAMIC; pDyn->d_tag != DT_NULL; pDyn ++) {
    switch (pDyn->d_tag) {
      case DT_RELA: rela_dyn = (void *)pDyn->d_un.d_ptr; break;
      case DT_RELASZ: nr_rela_dyn = pDyn->d_un.d_val / sizeof(Elf64_Rela); break;
    }
  }
  assert(rela_dyn != NULL && nr_rela_dyn != 0);

  for (i = 0; i < nr_rela_dyn; i ++) {
    if (ELF64_R_TYPE(rela_dyn[i].r_info) == R_X86_64_RELATIVE) {
      *(uintptr_t *)(rela_dyn[i].r_offset + PMEM_MAP_START) += -__am_rebase_offset;
    }
  }
#endif

  const char *args = getenv("mainargs");
  exit(main(args ? args : "")); // call main here!
}

static void exit_platform() __attribute__((destructor));
static void exit_platform() {
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
