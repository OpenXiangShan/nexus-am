#include <stdint.h>

struct ELFHeader {
	uint32_t magic;
	uint8_t  elf[12];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
};

struct ProgramHeader {
	uint32_t type;
	uint32_t off;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t filesz;
	uint32_t memsz;
	uint32_t flags;
	uint32_t align;
};

static inline char in_byte(short port) {
	char data;
	__asm__ volatile ("in %1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline int in_long(short port) {
  int data;
	__asm__ volatile ("in %1, %0" : "=a" (data) : "d" (port));
	return data;
}

static inline void out_byte(short port, char data) {
	__asm__ volatile ("out %0,%1" : : "a" (data), "d" (port));
}

static inline void hlt() {
  __asm__ volatile ("hlt");
}

#define SECTSIZE 512

void readseg(unsigned char *, int, int);

struct boot_info {
  int is_ap;
  void (*entry)();
};

void
bootmain(void) {
  volatile struct boot_info *boot = (void*)0x7000;
  int is_ap = boot->is_ap;
  if (is_ap == 1) {
    boot->entry();
  }
  struct ELFHeader *elf;
  struct ProgramHeader *ph, *eph;
  unsigned char *pa, *i;

  elf = (struct ELFHeader*)0x8000;

  readseg((unsigned char*)elf, 4096, 0);

  ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph ++) {
    pa = (unsigned char*)(ph->paddr);
    readseg(pa, ph->filesz, ph->off);
    for (i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
  }

  ((void(*)(void))elf->entry)();
}

void
waitdisk(void) {
  while ((in_byte(0x1F7) & 0xC0) != 0x40);
}

void
readsect(volatile void *dst, int offset) {
  int i;
  waitdisk();
  out_byte(0x1F2, 1);
  out_byte(0x1F3, offset);
  out_byte(0x1F4, offset >> 8);
  out_byte(0x1F5, offset >> 16);
  out_byte(0x1F6, (offset >> 24) | 0xE0);
  out_byte(0x1F7, 0x20);

  waitdisk();
  for (i = 0; i < SECTSIZE / 4; i ++) {
    ((int *)dst)[i] = in_long(0x1F0);
  }
}

void
readseg(unsigned char *pa, int count, int offset) {
  unsigned char *epa;
  epa = pa + count;
  pa -= offset % SECTSIZE;
  offset = (offset / SECTSIZE) + 1;
  for(; pa < epa; pa += SECTSIZE, offset ++)
    readsect(pa, offset);
}
