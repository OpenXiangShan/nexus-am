#include <sys/mman.h>
#include <sys/auxv.h>
#include <signal.h>
#include <fcntl.h>
#include <elf.h>
#include <stdlib.h>
#include <klib.h>

#define PMEM_SHM_FILE "/native-pmem"
#define PMEM_SIZE (128 * 1024 * 1024) // 128MB
#define PMEM_MAP_START (uintptr_t)0x100000
#define PMEM_MAP_END   (uintptr_t)PMEM_SIZE
#define PMEM_MAP_SIZE  (PMEM_MAP_END - PMEM_MAP_START)
#define REBASE(p) ((void *)(p) - aslr_offset + PMEM_MAP_START)

static int pmem_fd = 0;
static ucontext_t uc_example = {};

int main(const char *args);

static inline uintptr_t get_aslr_offset(Elf64_Phdr *phdr, int phnum) {
  int i;
  for (i = 0; i < phnum; i ++) {
    if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_W)) {
      extern char end;
      void *end_in_elf = (void *)(phdr[i].p_vaddr + phdr[i].p_memsz);
      uintptr_t offset = (void *)&end - end_in_elf;
      assert((offset & 0xfff) == 0);
      return offset;
    }
  }
  assert(0);
  return 0;
}

static void init_platform() __attribute__((constructor));
static void init_platform() {
  pmem_fd = shm_open(PMEM_SHM_FILE, O_RDWR | O_CREAT, 0700);
  assert(pmem_fd != -1);
  assert(0 == ftruncate(pmem_fd, PMEM_SIZE));

  void *ret = mmap((void *)PMEM_MAP_START, PMEM_MAP_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, pmem_fd, PMEM_MAP_START);
  assert(ret != (void *)-1);

  Elf64_Phdr *phdr = (void *)getauxval(AT_PHDR);
  int phnum = (int)getauxval(AT_PHNUM);
  uintptr_t aslr_offset = get_aslr_offset(phdr, phnum);

  extern char end;
  _heap.start = REBASE(&end);
  _heap.end = (void *)PMEM_MAP_END;

  getcontext(&uc_example);

  // block SIGVTALRM to simulate disabling interrupt
  int ret2 = sigaddset(&uc_example.uc_sigmask, SIGVTALRM);
  assert(ret2 == 0);

  // relocation, now we should not write any global variables
  // before calling the rebase verison of main(), else the update
  // can not be catched by the relocation process
  int i;
  for (i = 0; i < phnum; i ++) {
    if (phdr[i].p_type == PT_LOAD) {
      void *vaddr = (void *)(phdr[i].p_vaddr + aslr_offset);
      memcpy(REBASE(vaddr), vaddr, phdr[i].p_memsz);
    }
  }
  assert(*(int *)PMEM_MAP_START == 0x464c457f);

  const char *args = getenv("mainargs");
  int (*entry)(const char *) = REBASE(main);
  exit(entry(args ? args : "")); // call main here!
}

static void exit_platform() __attribute__((destructor));
static void exit_platform() {
  int ret = munmap((void *)PMEM_MAP_START, PMEM_MAP_SIZE);
  assert(ret == 0);
  close(pmem_fd);
  ret = shm_unlink(PMEM_SHM_FILE);
  assert(ret == 0);
}

void __am_shm_mmap(void *va, void *pa, int prot) {
  void *ret = mmap(va, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, pmem_fd, (uintptr_t)pa);
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
