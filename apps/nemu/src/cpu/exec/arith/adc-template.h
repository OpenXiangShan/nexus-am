#include "cpu/exec/template-start.h"
#include "cpu/exec/flag.h"

#define instr adc

static void do_execute(){
	DATA_TYPE result = op_src->val + op_dest->val + cpu.EFLAGS.CF;
	op_src->val = op_src->val + cpu.EFLAGS.CF;
	
	CHANGE_ZF
	CHANGE_SF
	CHANGE_PF
	CHANGE_ADD_OF
	CHANGE_ADD_CF
	OPERAND_W(op_dest, result);
	print_asm_template2();
}

make_instr_helper(r2rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

#include "cpu/exec/template-end.h"
