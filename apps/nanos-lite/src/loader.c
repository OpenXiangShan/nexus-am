#include "common.h"
#include "memory.h"
#include "x86.h"
#include <elf.h>

#define ELF_OFFSET_IN_DISK 0

#if defined(HAS_DEVICE) && defined(USE_HARDDISK)
# define driver_read ide_read
#else
# define driver_read ramdisk_read
#endif

void driver_read(uint8_t *, uint32_t, uint32_t);

#define STACK_SIZE (8 << 10)

void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader() {
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
      uint32_t va = mm_malloc(ph->p_vaddr, ph->p_memsz);
      driver_read((void *)va, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);


      /* TODO: zero the memory region
       * [VirtAddr + FileSiz, VirtAddr + MemSiz)
       */
      memset((void *)va + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);


#ifdef __PAGE
      /* Record the program break for future use. */
      extern uint32_t cur_brk, max_brk;
      uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
      if(cur_brk < new_brk) { max_brk = cur_brk = new_brk; }
#endif
    }
  }

  volatile uint32_t entry = elf->e_entry;

#ifdef __PAGE
  mm_malloc(0xc0000000 - STACK_SIZE, STACK_SIZE);

#ifdef HAS_DEVICE
  create_video_mapping();
#endif

  write_cr3(get_ucr3());
#endif

  return entry;
}
