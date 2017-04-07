#define _start (void *)0x0
#define _stack (void *)0xffffffff
#define SECTSIZE 512
#include "boot.h"

void bootmain(){
	struct ELFHeader *elf;
    	struct ProgramHeader *ph, *eph;
    	unsigned char *pa, *i;

    	elf = (struct ELFHeader*)_start;

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

void readseg(unsigned char *pa, int count, int offset) {
    	unsigned char *epa;
    	epa = pa + count;
    	pa -= offset % SECTSIZE;
    	offset = (offset / SECTSIZE) + 1;
    	for(; pa < epa; pa += SECTSIZE, offset ++)
        	readsect(pa, offset);
}
