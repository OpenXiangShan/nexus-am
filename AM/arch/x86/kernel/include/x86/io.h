#ifndef __X86_IO_H__
#define __X86_IO_H__
/* ELF32二进制文件头 */
struct ELFHeader {
	unsigned int   magic;
	unsigned char  elf[12];
	unsigned short type;
	unsigned short machine;
	unsigned int   version;
	unsigned int   entry;
	unsigned int   phoff;
	unsigned int   shoff;
	unsigned int   flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
};

/* ELF32 Program header */
struct ProgramHeader {
	unsigned int type;
	unsigned int off;
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int filesz;
	unsigned int memsz;
	unsigned int flags;
	unsigned int align;
};


static inline int 
in_long(short port) {
	int data;
	asm volatile("in %1, %0" : "=a" (data) : "d" (port));
	return data;
}

/* 读I/O端口 */
static inline uint8_t
in_byte(uint16_t port) {
	uint8_t data;
	asm volatile("in %1, %0" : "=a"(data) : "d"(port));
	return data;
}

/* 写I/O端口 */
static inline void
out_byte(uint16_t port, int8_t data) {
	asm volatile("out %%al, %%dx" : : "a"(data), "d"(port));
}

static inline void
out_byte2(short port, char data) {
	asm volatile("out %0,%1" : : "a" (data), "d" (port));
}
#endif
