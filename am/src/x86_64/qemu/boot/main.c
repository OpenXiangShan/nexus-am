#include <stdint.h>
#include <linux/elf.h>
#include <x86.h>

#define SECTSIZE 512

void load(void *paddr, int nbytes, int offset);

void load_kernel(void) {
  struct elf64_hdr *elf;
  struct elf64_phdr *ph, *eph;

  elf = (struct elf64_hdr *)0x8000;

  if (!boot_record()->is_ap) {
    load(elf, 4096, 0);
  
    ph = (struct elf64_phdr *)((char *)elf + elf->e_phoff);
    eph = ph + elf->e_phnum;
  
    for(; ph < eph; ph ++) {
      uint32_t filesz = (uint32_t)ph->p_filesz;
      uint32_t memsz =  (uint32_t)ph->p_memsz;
      void *paddr = (void *)((uint32_t)ph->p_paddr);
      load(paddr, ph->p_filesz, ph->p_offset);
  
      char *bss = paddr + filesz;
      for (uint32_t i = filesz; i != memsz; i++) {
        *bss++ = 0;
      }
    }
  
    char *mainargs = (void *)0x7e00;
    load(mainargs, 512, -512);
  } else {
  }
  ((void(*)())(uint32_t)elf->e_entry)();
}

void waitdisk(void) {
  while ((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *ptr, int sect) {
  waitdisk();
  outb(0x1f2, 1);
  outb(0x1f3, sect);
  outb(0x1f4, sect >> 8);
  outb(0x1f5, sect >> 16);
  outb(0x1f6, (sect >> 24) | 0xE0);
  outb(0x1f7, 0x20);
  waitdisk();
  for (int i = 0; i < SECTSIZE / 4; i ++) {
    ((uint32_t *)ptr)[i] = inl(0x1f0);
  }
}

void load(void *paddr, int nbytes, int offset) {
  unsigned char *cur = paddr;
  unsigned char *ed = paddr + nbytes;
  cur -= offset % SECTSIZE;
  int sect = (offset / SECTSIZE) + 1 + 1 /* args */;
  for(; cur < ed; cur += SECTSIZE, sect ++)
    readsect(cur, sect);
}
