#include "common.h"
#include "memory.h"
#include <elf.h>

#define ELF_OFFSET_IN_DISK 0

#if defined(HAS_DEVICE) && defined(USE_HARDDISK)
# define driver_read ide_read
#else
# define driver_read ramdisk_read
#endif

void driver_read(uint8_t *, uint32_t, uint32_t);

#define STACK_SIZE (2 * PGSIZE)

uint32_t loader(_Protect *p) {
  Elf32_Ehdr *elf;
  Elf32_Phdr *ph = NULL, *eph;

  uint8_t buf[4096];
  driver_read(buf, ELF_OFFSET_IN_DISK, 4096);

  elf = (void*)buf;

  /* TODO: fix the magic number with the correct one */
  const uint32_t elf_magic = 0x464c457f; //0xbadc0df;
  uint32_t *p_magic = (void *)buf;
  assert(*p_magic == elf_magic);

  /* Load each program segment */
  //TODO();
  for(ph = (void *)buf + elf->e_phoff, eph = ph + elf->e_phnum; ph < eph; ph ++) {
    /* Scan the program header table, load each segment into memory */
    if(ph->p_type == PT_LOAD && ph->p_memsz != 0) {

      /* TODO: read the content of the segment from the ELF file
       * to the memory region [VirtAddr, VirtAddr + FileSiz)
       */
      void *va_align = (void *)(ph->p_vaddr & ~PGMASK);
      uint32_t size = ph->p_memsz + (ph->p_vaddr - (uint32_t)va_align);
      void *pa = kmalloc(size);
      uint32_t i;
      for (i = 0; i < size; i += PGSIZE) {
        _map(p, va_align + i, pa + i);
      }

      driver_read((void *)pa, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);


      /* TODO: zero the memory region
       * [VirtAddr + FileSiz, VirtAddr + MemSiz)
       */
      memset((void *)pa + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);


#ifdef __PAGE
      /* Record the program break for future use. */
      extern uint32_t cur_brk, max_brk;
      uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
      if(cur_brk < new_brk) { max_brk = cur_brk = new_brk; }
#endif
    }
  }

#ifdef __PAGE
  void *va = (void *)(0xc0000000 - STACK_SIZE);
  while(va < (void *)0xc0000000) {
    _map(p, va, new_page());
    va += PGSIZE;
  }
#endif

  return elf->e_entry;
}
