#include "cpu/exec/template-start.h"
#include "cpu/exec/flag.h"

#define instr cmp

static void do_execute(){
	DATA_TYPE result = op_dest->val - (DATA_TYPE_S)op_src->val;
	CHANGE_ZF
	CHANGE_SF
	CHANGE_PF
	CHANGE_SUB_OF
	CHANGE_SUB_CF
	print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"
