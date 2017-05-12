#include "cpu/exec/template-start.h"

#define instr movzx

static void do_execute(){
	if(instr_fetch(cpu.eip+1,1) == 0xb6)/* movzb */
	{
		uint8_t result = (DATA_TYPE)op_src->val;
		OPERAND_W(op_dest,result);
	}
	else
	{
		uint16_t result = (DATA_TYPE)op_src->val;
		OPERAND_W(op_dest,result);
	}
	print_asm_template2();
}

make_instr_helper(rm2r);

#include "cpu/exec/template-end.h"
