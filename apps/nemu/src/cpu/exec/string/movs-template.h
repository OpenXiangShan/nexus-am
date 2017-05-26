#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_,SUFFIX)) {
	DATA_TYPE temp =swaddr_read(cpu.esi,DATA_BYTE,R_ES);
	swaddr_write(cpu.edi,DATA_BYTE,temp,R_ES);
	if(cpu.EFLAGS.DF == 0) 
	{
		cpu.esi += DATA_BYTE;
		cpu.edi += DATA_BYTE;
	}
	else 
	{
		cpu.esi -= DATA_BYTE;
		cpu.edi -= DATA_BYTE;
	}
	print_asm("movs"str(SUFFIX));
	return 1;
}

#include "cpu/exec/template-end.h"
