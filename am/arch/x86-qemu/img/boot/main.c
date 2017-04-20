#include "boot.h"

#define SECTSIZE 512

void readseg(unsigned char *, int, int);

void
bootmain(void) {
    // multi-core boot hack, should use better ways.
    if (*(unsigned short*)(0x7c00 + 510) == 0x55aa) {
      ((void(*)(void))*(void**)0x7010)();
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
