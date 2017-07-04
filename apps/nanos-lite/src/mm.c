#include "memory.h"


void* new_page(void) {
  assert(pf < (void *)PMEM_SIZE);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

void* kmalloc(uint32_t size) {
  assert(size > 0);

  void *pa = new_page();
  uint32_t alloc_size = PGSIZE;
  for (; alloc_size < size; alloc_size += PGSIZE) {
    new_page();
  }

  return pa;
}

uint32_t cur_brk = 0;
// we do not free memory, so use `max_brk' to determine whether to call mm_malloc()
uint32_t max_brk = 0;

/* The brk() system call handler. */
uint32_t mm_brk(uint32_t new_brk) {
  panic("not implement yet");
	if(new_brk != 0) {
		if(new_brk > max_brk) {
#ifdef IA32_PAGE
//			mm_malloc(max_brk, new_brk - max_brk);
#endif
			max_brk = new_brk;
		}

		cur_brk = new_brk;
	}

	// If new_brk == 0, the brk() syscall should fail and return
	// the current break. See the NOTE of `man 2 brk' for details.

	return cur_brk;
}

void init_mm() {
  extern char _end;
  pf = (void *)PGROUNDUP((uintptr_t)&_end);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
