#ifndef mips_h
#define mips_h

#define KERNEL_MODE 0
#define USER_MODE 3

#define VMEM_ADDR 0xc0000000
#define KMEM_ADDR 0xe0000000

typedef struct CP0 {
	unsigned int index,entrylo0,entrylo1,
		     context,pagemask,badvaddr,
		     count,entryhi,compare,status,
		     cause,epc,prid,watchlo,watchhi,
		     taglo,taghi;
} CP0;
#endif
