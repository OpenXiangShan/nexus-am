#include "x86.h"
#include "device.h"
#include "common.h"
#define SYS_write 4

void do_syscall(struct TrapFrame *tf);
extern int write(int fd, char *buf, int len, int line, int row);

void
irq_handle(struct TrapFrame *tf) {
    	switch(tf->irq) {
		case 0x80:do_syscall(tf);break;
		case 3:printk("hit breakpoint exception(3)\n");while(1);break;
		case 13:printk("hit protect exception(13)\n");while(1);break;
		case 1000:assert(0);break;
		case 1001:assert(0);break;
		case 1014:assert(0);break;
        	default:printk("irq = %d\n",tf->irq);assert(0);
    	}
}

void
do_syscall(struct TrapFrame *tf) {
	switch(tf->eax)
	{
		case SYS_write:tf->eax = write(tf->edx,(void *)tf->ecx,tf->ebx,tf->edi,tf->esi);break;
		default: assert(0);
	}
}

