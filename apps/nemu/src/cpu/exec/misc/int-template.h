#include "cpu/exec/template-start.h"

#define instr int

static void do_execute() {
	assert(cpu.cr0.protect_enable == 1);
	raise_intr(op_src->val);
	print_asm_template1();
}

make_instr_helper(i)

