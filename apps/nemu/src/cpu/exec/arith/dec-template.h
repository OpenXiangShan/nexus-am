#include "cpu/exec/template-start.h"
#include "cpu/exec/flag.h"

#define instr dec

static void do_execute () {
	op_dest->val = op_src->val;
	DATA_TYPE result = op_src->val - 1;
	op_src->val = 1;
	CHANGE_ZF
	CHANGE_SF
	CHANGE_PF
	CHANGE_SUB_OF
	CHANGE_SUB_CF
	OPERAND_W(op_src, result);
	print_asm_template1();
}

make_instr_helper(rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
#endif

#include "cpu/exec/template-end.h"
