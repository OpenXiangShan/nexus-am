#include <stdio.h>
#include <draw.h>

int main(){
	//printf("Hello,world\n");
	printf("Hello,world");
	int i = 0;
	short *vmem = (void *)0xb8000;
	for(i = 0; i < 100; i++)
		vmem[i] = 'h' + 0x0d00;
	while(1);
	return 0;
}
