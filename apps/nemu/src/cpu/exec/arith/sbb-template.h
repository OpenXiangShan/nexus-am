#include "cpu/exec/template-start.h"
#include "cpu/exec/flag.h"

#define instr sbb

static void do_execute(){
	DATA_TYPE result = op_dest->val - ((DATA_TYPE_S)op_src->val + cpu.EFLAGS.CF);
	op_src->val = (DATA_TYPE_S)op_src->val + cpu.EFLAGS.CF;
	CHANGE_ZF
	CHANGE_SF
	CHANGE_PF
	CHANGE_SUB_OF
	CHANGE_SUB_CF
	OPERAND_W(op_dest,result);
	print_asm_template2();
}

make_instr_helper(rm2r)
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"
