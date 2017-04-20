#include "boot.h"

#define SECTSIZE 512

void readdisk(unsigned char *elf, int count, int offset);

void bootmain(void)
{
	struct ELFHeader *elf;

	/* 这里是加载磁盘程序的代码 */
	struct ProgramHeader *ph, *eph;
	unsigned char * pa, *i;
	elf = (struct ELFHeader *)0x8000;
	readdisk((void *)elf, 4096, 0);
	
	ph = (struct ProgramHeader *)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(;ph < eph; ph++)
	{
		pa = (unsigned char *)ph->paddr;
		readdisk(pa, ph->filesz, ph->off);
		for(i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
	}
	((void(*)(void))elf->entry)();	//goto kernel main
}

void
waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0x40); /* 等待磁盘完毕 */
}

/* 读磁盘的一个扇区 */
void
readsect(void *dst, int offset) {
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

void readdisk(unsigned char *elf, int count, int offset) {
	unsigned char * eelf;
	eelf = elf + count;
	elf -= offset % SECTSIZE;
	offset = offset / SECTSIZE + 1;
	for(; elf < eelf; elf += SECTSIZE, offset++)
		readsect(elf,offset);
}
