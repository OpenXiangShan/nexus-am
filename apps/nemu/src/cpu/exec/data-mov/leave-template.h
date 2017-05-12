#include "cpu/exec/template-start.h"

#define instr leave

make_helper(concat(leave_none_ , SUFFIX)){
	cpu.esp = cpu.ebp;
	cpu.ebp = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp = cpu.esp + 4;
	print_asm(str(instr));
	return 1;
}

#include "cpu/exec/template-end.h"
