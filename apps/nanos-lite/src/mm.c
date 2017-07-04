#include "memory.h"

static void *pf = NULL;
static _Protect user_as;  // user process address space
_Protect* get_user_as(void) {
  return &user_as;
}

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

static uintptr_t cur_brk = 0;
// we do not free memory, so use `max_brk' to determine whether to call mm_malloc()
static uintptr_t max_brk = 0;

/* The brk() system call handler. */
bool mm_brk(uint32_t new_brk) {
	if(cur_brk == 0) {
    cur_brk = max_brk = new_brk;
  }
  else {
		if (new_brk > max_brk) {
      uintptr_t page_start = PGROUNDUP(max_brk);
      uintptr_t page_end = PGROUNDUP(new_brk);
      for (; page_start <= page_end; page_start += PGSIZE) {
        _map(&user_as, (void *)page_start, new_page());
      }
			max_brk = new_brk;
		}
		cur_brk = new_brk;
	}

  // success
  return 0;
}

void init_mm() {
  extern char _end;
  pf = (void *)PGROUNDUP((uintptr_t)&_end);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
