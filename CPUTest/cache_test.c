#include "trap.h"
#include "stdio.h"
#define VMEM ((char *)0xc0000000)
int main()
{	
	 char* vga = VMEM + 320 + 80;
	volatile int * x = (int *)0x01000000;
	*x = 0x0c0c0c0c;
	x = (int *)0x02000000;
	*x = 0xffff0c0c;
	x = (int *)0x03000000;
	*x = 0xffffffff;
	x = (int *)0x04000000;
	*x = 0x0c0cffff;
	x = (int *)0x05000000;
	*x = 0x77777777;
	x = (int *)0x06000000;
	*x = 0x55555555;
	if(*((int *)0x01000000) != 0x0c0c0c0c) {
		bad();
	}

	if(*((int *)0x02000000) != 0xffff0c0c) {
		bad();
		
	}

	if(*((int *)0x03000000) != 0xffffffff) {
		bad();
	}

	if(*((int *)0x04000000) != 0x0c0cffff) {
		bad();
	}

	if(*((int *)0x05000000) != 0x77777777) {
		bad();
	}

	if(*((int *)0x06000000) != 0x55555555) {
		bad();
	}

	put_hex(*((int *)0x01000000), vga);
	vga += 160;
	
	put_hex(*((int *)0x02000000), vga);
	vga += 160;

	put_hex(*((int *)0x03000000), vga);
	vga += 160;

	put_hex(*((int *)0x04000000), vga);
	vga += 160;

	put_hex(*((int *)0x05000000), vga);
	vga += 160;

	put_hex(*((int *)0x06000000), vga);
	vga += 160;
	good();

	return good();;
}
