#include <sys/mman.h>
#include <sys/auxv.h>
#include <fcntl.h>
#include <elf.h>
#include <stdlib.h>
#include "platform.h"

#define MAX_CPU 16
#define TRAP_PAGE_START (void *)0x100000
#define PMEM_SIZE (128 * 1024 * 1024) // 128MB
static int pmem_fd = 0;
static char pmem_shm_file[] = "/native-pmem-XXXXXX";
static void *pmem = NULL;
static ucontext_t uc_example = {};
sigset_t __am_intr_sigmask = {};
__am_cpu_t *__am_cpu_struct = NULL;
int __am_ncpu = 0;

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

  // allocate private per-cpu structure
  thiscpu = mmap(NULL, sizeof(*thiscpu), PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(thiscpu != (void *)-1);
  thiscpu->cpuid = 0;
  thiscpu->cur_as = NULL;

  // create trap page to receive syscall and yield by SIGSEGV
  void *ret = mmap(TRAP_PAGE_START, 4096, PROT_NONE,
      MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
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
      ret = mmap_libc(vaddr_align, size, PROT_READ | PROT_WRITE | PROT_EXEC,
          MAP_SHARED | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
      assert(ret == vaddr_align);

      // restore the data in the sections
      memcpy_libc(vaddr_align, temp_mem, size);

      // unmap the temporary memory
      ret2 = munmap(temp_mem, size);
      assert(ret2 == 0);
    }
  }

  // set up the AM heap
  _heap = RANGE(pmem, pmem + PMEM_SIZE);

  // initialize sigmask for interrupts
  ret2 = sigemptyset(&__am_intr_sigmask);
  assert(ret2 == 0);
  ret2 = sigaddset(&__am_intr_sigmask, SIGVTALRM);
  assert(ret2 == 0);
  ret2 = sigaddset(&__am_intr_sigmask, SIGUSR1);
  assert(ret2 == 0);

  // disable interrupts by default
  _intr_write(0);

  // save the context template
  getcontext(&uc_example);
  __am_get_intr_sigmask(&uc_example.uc_sigmask);

  // set ncpu
  char *smp = getenv("smp");
  __am_ncpu = smp ? atoi(smp) : 1;
  assert(0 < __am_ncpu && __am_ncpu <= MAX_CPU);

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

  ret = munmap(thiscpu, sizeof(*thiscpu));
  assert(ret == 0);

  ret = munmap(TRAP_PAGE_START, 4096);
  assert(ret == 0);
}

void __am_shm_mmap(void *va, void *pa, int prot) {
  // translate AM prot to mmap prot
  int mmap_prot = PROT_NONE;
  // we do not support executable bit, so mark
  // all readable pages executable as well
  if (prot | _PROT_READ) mmap_prot |= PROT_READ | PROT_EXEC;
  if (prot | _PROT_WRITE) mmap_prot |= PROT_WRITE;
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

void __am_get_intr_sigmask(sigset_t *s) {
  memcpy(s, &__am_intr_sigmask, sizeof(__am_intr_sigmask));
}

int __am_is_sigmask_sti(sigset_t *s) {
  return !sigismember(s, SIGVTALRM);
}

// This dummy function will be called in trm.c.
// The purpose of this dummy function is to let linker add this file to the object
// file set. Without it, the constructor of @_init_platform will not be linked.
void __am_platform_dummy() {
}
