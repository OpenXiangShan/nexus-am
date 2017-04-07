#ifndef mips_h
#define mips_h

#define KERNEL_MODE 0
#define USER_MODE 3

#define VMEM_ADDR 0xc0000000
#define SERIAL_PORT 0xe0000000
#define Rx_FIFO 0x0
#define Tx_FIFO 0x04
#define STAT_REG 0x08
#define CTRL_REG 0x0c

typedef struct CP0 {
	unsigned int index,entrylo0,entrylo1,
		     context,pagemask,badvaddr,
		     count,entryhi,compare,status,
		     cause,epc,prid,watchlo,watchhi,
		     taglo,taghi;
} CP0;
#endif
