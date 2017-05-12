#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute(){
	uint32_t ret = read_ModR_M(cpu.eip + 1,op_src,op_src2) + 1;
	if(DATA_BYTE == 2)
		cpu.eip = (op_src->val & 0x0000FFFF) - ret;
	else
		cpu.eip = op_src->val - ret;
	print_asm_template1();
}

make_helper(concat(jmp_rel_,SUFFIX)){
	DATA_TYPE_S disp = instr_fetch(eip + 1, DATA_BYTE);
	cpu.eip = eip + disp;
	print_asm(str(instr) " %x", cpu.eip + DATA_BYTE + 1);
	return DATA_BYTE + 1;
}

make_helper(concat(ljmp_ptr_,SUFFIX)) {
	if(DATA_BYTE == 4)
	{
		cpu.eip = instr_fetch(cpu.eip + 1,4) - 7;
		cpu.CS.val = instr_fetch(cpu.eip + 5,2);
		print_asm("ljmp " "$0x%x,$0x%x",cpu.CS.val,cpu.eip + 7);
		return 7;
	}
	else
	{
		cpu.CS.val = instr_fetch(cpu.eip + 3,2);
		cpu.eip = (cpu.eip & 0xFFFF0000) + instr_fetch(cpu.eip + 1,2) - 5;
		cpu.eip = cpu.eip & 0x0000FFFF;
		print_asm("ljmp " "$0x%x,$0x%x",cpu.CS.val,cpu.eip + 5);
		return 5;
	}
}

make_instr_helper(rm)


#include "cpu/exec/template-end.h"
