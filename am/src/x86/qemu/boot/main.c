#include <stdint.h>
#include <linux/elf.h>
#include <linux/elf-em.h>
#include <x86.h>

#define SECTSIZE 512

static inline void wait_disk(void) {
  while ((inb(0x1f7) & 0xc0) != 0x40);
}

static inline void read_disk(void *buf, int sect) {
  wait_disk();
  outb(0x1f2, 1);
  outb(0x1f3, sect);
  outb(0x1f4, sect >> 8);
  outb(0x1f5, sect >> 16);
  outb(0x1f6, (sect >> 24) | 0xE0);
  outb(0x1f7, 0x20);
  wait_disk();
  for (int i = 0; i < SECTSIZE / 4; i ++) {
    ((uint32_t *)buf)[i] = inl(0x1f0);
  }
}

static inline void copy_from_disk(void *buf, int nbytes, int disk_offset) {
  uint32_t cur  = (uint32_t)buf & ~511;
  uint32_t ed   = (uint32_t)buf + nbytes;
  uint32_t sect = (disk_offset >> 9) + 3;
  for(; cur < ed; cur += SECTSIZE, sect ++)
    read_disk((void *)cur, sect);
}

static void load_program(uint32_t filesz, uint32_t memsz, uint32_t paddr, uint32_t offset) {
  copy_from_disk((void *)paddr, filesz, offset);
  char *bss = (void *)(paddr + filesz);
  for (uint32_t i = filesz; i != memsz; i++) {
    *bss++ = 0;
  }
}

static void load_elf64(struct elf64_hdr *elf) {
  struct elf64_phdr *ph = (struct elf64_phdr *)((char *)elf + elf->e_phoff);
  for (int i = 0; i < elf->e_phnum; i++, ph++) {
    load_program(
      (uint32_t)ph->p_filesz,
      (uint32_t)ph->p_memsz,
      (uint32_t)ph->p_paddr,
      (uint32_t)ph->p_offset
    );
  }
}

static void load_elf32(struct elf32_hdr *elf) {
  struct elf32_phdr *ph = (struct elf32_phdr *)((char *)elf + elf->e_phoff);
  for (int i = 0; i < elf->e_phnum; i++, ph++) {
    load_program(
      (uint32_t)ph->p_filesz,
      (uint32_t)ph->p_memsz,
      (uint32_t)ph->p_paddr,
      (uint32_t)ph->p_offset
    );
  }
}

void load_kernel(void) {
  struct elf32_hdr *elf32 = (void *)0x8000;
  struct elf64_hdr *elf64 = (void *)0x8000;
  int is_ap = boot_record()->is_ap;

  if (!is_ap) {
    // load argument (string) to memory
    copy_from_disk((void *)MAINARG_ADDR, 1024, -1024);
    // load elf header to memory
    copy_from_disk(elf32, 4096, 0);
    if (elf32->e_machine == EM_X86_64) {
      load_elf64(elf64);
    } else {
      load_elf32(elf32);
    }
  } else {
    // everything should be loaded
  }

  if (elf32->e_machine == EM_X86_64) {
    ((void(*)())(uint32_t)elf64->e_entry)();
  } else {
    ((void(*)())(uint32_t)elf32->e_entry)();
  }
}
