#include "cpu/exec/template-start.h"

static void concat(do_cmova_,SUFFIX)() {
	if(cpu.EFLAGS.ZF == 0 && cpu.EFLAGS.CF == 0)
		OPERAND_W(op_dest,op_src->val);
	print_asm_template2();
}

static void concat(do_cmove_,SUFFIX)() {
	if(cpu.EFLAGS.ZF == 1)
		OPERAND_W(op_dest,op_src->val);
	print_asm_template2();
}

static void concat(do_cmovge_,SUFFIX)() {
	if(cpu.EFLAGS.SF == cpu.EFLAGS.OF)
		OPERAND_W(op_dest,op_src->val);
	print_asm_template2();
}

static void concat(do_cmovne_,SUFFIX)() {
	if(cpu.EFLAGS.ZF == 0)
		OPERAND_W(op_dest,op_src->val);
	print_asm_template2();
}

static void concat(do_cmovns_,SUFFIX)() {
	if(cpu.EFLAGS.SF == 0)
		OPERAND_W(op_dest,op_src->val);
	print_asm_template2();
}

static void concat(do_cmovs_,SUFFIX)() {
	if(cpu.EFLAGS.SF == 1)
		OPERAND_W(op_dest,op_src->val);
	print_asm_template2();
}

#define instr cmova
make_instr_helper(rm2r)
#undef instr

#define instr cmove
make_instr_helper(rm2r)
#undef instr

#define instr cmovge
make_instr_helper(rm2r)
#undef instr

#define instr cmovne
make_instr_helper(rm2r)
#undef instr

#define instr cmovns
make_instr_helper(rm2r)
#undef instr

#define instr cmovs
make_instr_helper(rm2r)
#undef instr

#include "cpu/exec/template-end.h"
