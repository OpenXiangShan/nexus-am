#include "cpu/exec/template-start.h"
#include "cpu/exec/flag.h"

#define instr add

static void do_execute(){
	DATA_TYPE result = op_dest->val + op_src->val;
	CHANGE_ZF
	CHANGE_SF
	CHANGE_PF
	CHANGE_ADD_OF
	CHANGE_ADD_CF
	OPERAND_W(op_dest, result);
	print_asm_template2();
}

make_instr_helper(rm2r)
make_instr_helper(r2rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
make_instr_helper(i2a)
make_instr_helper(i2rm)

#include "cpu/exec/template-end.h"
