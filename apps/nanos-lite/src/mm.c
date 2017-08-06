#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)PMEM_SIZE);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
bool mm_brk(uint32_t new_brk) {
	if(current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
		if (new_brk > current->max_brk) {
      uintptr_t page_start = PGROUNDUP(current->max_brk);
      uintptr_t page_end = PGROUNDUP(new_brk);
      for (; page_start <= page_end; page_start += PGSIZE) {
        _map(&current->as, (void *)page_start, new_page());
      }
			current->max_brk = new_brk;
		}
		current->cur_brk = new_brk;
	}

  // success
  return 0;
}

void init_mm() {
  _pte_init(new_page, free_page);

  extern char _end;
  pf = (void *)PGROUNDUP((uintptr_t)&_end);
  Log("free physical pages starting from %p", pf);
}
