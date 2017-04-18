#include "x86.h"
#include "device.h"

#define SECTSIZE 512
#define ELF_OFFSET_DISK 200 * SECTSIZE

void readdisk(unsigned char *elf, int count, int offset);
struct ELFHeader *elf;
uint8_t buf[4096];

uint32_t
load_umain(void) {
    /*
     * Load your app here
     * 加载用户程序
     */
	struct ProgramHeader *ph, *eph;
	unsigned char * pa, *i;
	elf = (void *)buf;
	readdisk((void *)elf, 4096, ELF_OFFSET_DISK);
	
	ph = (struct ProgramHeader *)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(;ph < eph; ph++)
	{
		if(ph->type == 1)
		{
			pa = (unsigned char *)ph->paddr + USER_CS_BASE;
			readdisk(pa, ph->filesz, ph->off + ELF_OFFSET_DISK);
			for(i = pa + ph->filesz; i < pa + ph->memsz; *i++ = 0);
		}
	}
	return elf->entry;
}


void
waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0x40); 
}

void
readsect(void *dst, int offset) {
	int i;
	waitdisk();
	out_byte2(0x1F2, 1);
	out_byte2(0x1F3, offset);
	out_byte2(0x1F4, offset >> 8);
	out_byte2(0x1F5, offset >> 16);
	out_byte2(0x1F6, (offset >> 24) | 0xE0);
	out_byte2(0x1F7, 0x20);

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
