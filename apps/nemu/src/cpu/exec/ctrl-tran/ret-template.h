#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_none_, SUFFIX)){
	cpu.eip = swaddr_read(cpu.esp, DATA_BYTE, R_CS);
	cpu.esp = cpu.esp + DATA_BYTE;
	if(DATA_BYTE == 2)
		cpu.eip = cpu.eip & 0x0000FFFF;
	print_asm(str(instr));
	return 1;
}

make_helper(concat(ret_i_,SUFFIX)){
	cpu.eip = swaddr_read(cpu.esp, DATA_BYTE, R_CS);
	int16_t disp = instr_fetch(cpu.eip + 1, 1);
	cpu.esp = cpu.esp + DATA_BYTE + disp;
	cpu.eip = cpu.eip - 1;
	print_asm(str(instr) " 0x%x", disp);
	return 2;
}

#include "cpu/exec/template-end.h"
