#include "common.h"
#include "mips.h"
#include "device.h"

void enter_user_space(unsigned int);
extern int main();

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
    	unsigned int entry = load_umain();
	printk("leaving kernel, entering user space!\n");
    	enter_user_space(entry);
	printk("should not reach here!\n");
}

void
enter_user_space(unsigned int entry) {
	asm volatile("jal main\n\t");
	while(1);
}
