#include "cpu/exec/template-start.h"

#define instr stos

#if DATA_BYTE == 1
make_helper(stosb){
	swaddr_write(cpu.edi,1,(cpu.eax & 0xFF), R_ES);
	if(cpu.EFLAGS.DF == 0)
		cpu.edi+=1;
	else
		cpu.edi-=1;
	print_asm("stosb");
	return 1;
}

#endif

#if DATA_BYTE == 4
make_helper(stosv){
	swaddr_write(cpu.edi,4,cpu.eax, R_ES);
	if(cpu.EFLAGS.DF == 0)
		cpu.edi+=4;
	else
		cpu.edi-=4;
	print_asm("stosv");
	return 1;
}
#endif
#include "cpu/exec/template-end.h"
