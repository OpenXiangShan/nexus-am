#include <stdint.h>
#include <linux/elf.h>
#include <x86.h>

#define SECTSIZE     512
#define KERNEL_BASE  0x00100000

void readseg(void *, int, int);

void bootmain(void) {
  struct elf64_hdr *elf;
  struct elf64_phdr *ph, *eph;

  elf = (struct elf64_hdr *)0x8000;
  readseg(elf, 4096, 0);

  ph = (struct elf64_phdr *)((char *)elf + elf->e_phoff);
  eph = ph + elf->e_phnum;

  for(; ph < eph; ph ++) {
    uint8_t *paddr = (void *)((uintptr_t)(ph->p_paddr) + KERNEL_BASE);
    readseg(paddr, ph->p_filesz, ph->p_offset);

    char *ptr = paddr + (uintptr_t)ph->p_filesz;
    while (ph->p_memsz--) {
      *ptr++ = 0;
    }
  }

  char *mainargs = (void *)0x7e00;
  readseg(mainargs, 512, -512);
  ((void(*)())(uint32_t)elf->e_entry + KERNEL_BASE)();
}

void waitdisk(void) {
  while ((inb(0x1F7) & 0xC0) != 0x40);
}

void readsect(volatile void *dst, int offset) {
  waitdisk();
  outb(0x1f2, 1);
  outb(0x1f3, offset);
  outb(0x1f4, offset >> 8);
  outb(0x1f5, offset >> 16);
  outb(0x1f6, (offset >> 24) | 0xE0);
  outb(0x1f7, 0x20);

  waitdisk();
  for (int i = 0; i < SECTSIZE / 4; i ++) {
    ((int *)dst)[i] = inl(0x1F0);
  }
}

void readseg(void *addr, int count, int offset) {
  unsigned char *pa = addr;
  unsigned char *epa = pa + count;
  pa -= offset % SECTSIZE;
  offset = (offset / SECTSIZE) + 1 + 1 /* args */;
  for(; pa < epa; pa += SECTSIZE, offset ++)
    readsect(pa, offset);
}
