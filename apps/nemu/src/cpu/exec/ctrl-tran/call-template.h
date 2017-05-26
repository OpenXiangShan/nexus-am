#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_rel_, SUFFIX)){
	uint32_t disp = instr_fetch(eip + 1, DATA_BYTE);
	cpu.esp = cpu.esp - 4;
	swaddr_write(cpu.esp, 4, eip + DATA_BYTE, R_CS);
	cpu.eip = eip + disp;
	print_asm("call " "%x", cpu.eip + DATA_BYTE + 1);
	return DATA_BYTE + 1;
}

static void do_execute(){
	cpu.esp = cpu.esp - 4;
	uint32_t ret = read_ModR_M(cpu.eip + 1,op_src,op_src2);
	swaddr_write(cpu.esp, 4, cpu.eip + ret, R_CS);
	cpu.eip = op_src->val - ret - 1;
	print_asm_template1();
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"
