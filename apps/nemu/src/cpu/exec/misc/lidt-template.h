#include "cpu/exec/template-start.h"

#define instr lidt

static void do_execute() {
	cpu.IDTR.IDT_LIMIT = swaddr_read(op_src->addr, 2, R_CS);
	cpu.IDTR.IDT_BASE = swaddr_read(op_src->addr + 2, 4, R_CS);
	print_asm_template1();
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"
