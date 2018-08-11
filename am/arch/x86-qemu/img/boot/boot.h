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
