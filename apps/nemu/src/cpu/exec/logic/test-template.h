#include "cpu/exec/template-start.h"
#include "cpu/exec/flag.h"

#define instr test

static void do_execute(){
	DATA_TYPE result = op_dest->val & op_src->val;
	CHANGE_ZF
	CHANGE_SF
	CHANGE_PF
	cpu.EFLAGS.CF = 0;
	cpu.EFLAGS.OF = 0;
	print_asm_template2();
}

make_instr_helper(r2rm)
make_instr_helper(i2rm)
make_instr_helper(i2a)

#include "cpu/exec/template-end.h"
