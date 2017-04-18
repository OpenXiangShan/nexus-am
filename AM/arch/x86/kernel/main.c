#include "common.h"
#include "x86.h"
#include "device.h"

void enter_user_space(uint32_t);

void
kentry(void) {
	printk("Hello, welcome to kernel!\n");
	init_serial();
	printk("serial initialization complete!\n");
    	init_idt();
	printk("idt initialization complete!\n");
	init_intr();
	printk("interrupt initialization complete!\n");
    	init_seg();
	printk("segment initialization complete!\n");
	printk("loading files...\n");
    	uint32_t entry = load_umain();
	printk("leaving kernel, entering user space!\n");
    	enter_user_space(entry);
	printk("should not reach here!\n");
	assert(0);
}

void
enter_user_space(uint32_t entry) {
    /*
     * Before enter user space 
     * you should set the right segment registers here
     * and use 'iret' to jump to ring3
     * 进入用户空间
     */
	asm volatile("movw %%ax,%%es":: "a" (USEL(SEG_UDATA)));
	asm volatile("movw %%ax,%%ds":: "a" (USEL(SEG_UDATA)));

	asm volatile("pushl %0		\n\t"
		     "pushl %1		\n\t"
		     "pushl $0x2	\n\t"
		     "pushl %2		\n\t"
		     "pushl %3		\n\t"
		     "iret		\n\t"
			::"i"(USEL(SEG_UDATA)),
			  "i"(STACK),
			  "i"(USEL(SEG_UCODE)),
			  "g"(entry));
}
