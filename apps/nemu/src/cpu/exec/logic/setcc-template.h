#include "cpu/exec/template-start.h"

static void concat(do_setne_,SUFFIX)(){
	if(cpu.EFLAGS.ZF == 0)
		OPERAND_W(op_dest, 1);
	else
		OPERAND_W(op_dest, 0);
	print_asm_template1();
}

static void concat(do_sete_,SUFFIX)(){
	if(cpu.EFLAGS.ZF == 1)
		OPERAND_W(op_dest, 1);
	else
		OPERAND_W(op_dest, 0);
	print_asm_template1();
}

static void concat(do_setge_,SUFFIX)(){
	if(cpu.EFLAGS.SF == cpu.EFLAGS.OF)
		OPERAND_W(op_dest, 1);
	else
		OPERAND_W(op_dest, 0);
	print_asm_template1();
}

static void concat(do_setle_,SUFFIX)(){
	if(cpu.EFLAGS.ZF == 1 || cpu.EFLAGS.SF != cpu.EFLAGS.OF)
		OPERAND_W(op_dest, 1);
	else
		OPERAND_W(op_dest, 0);
	print_asm_template1();
}

static void concat(do_seta_,SUFFIX)(){
	if(cpu.EFLAGS.CF == 0 && cpu.EFLAGS.ZF == 0)
		OPERAND_W(op_dest, 1);
	else
		OPERAND_W(op_dest, 0);
	print_asm_template1();
}

static void concat(do_setl_,SUFFIX)(){
	if(cpu.EFLAGS.SF != cpu.EFLAGS.OF)
		OPERAND_W(op_dest, 1);
	else
		OPERAND_W(op_dest, 0);
	print_asm_template1();
}

static void concat(do_setg_,SUFFIX)(){
	if(cpu.EFLAGS.SF == cpu.EFLAGS.OF || cpu.EFLAGS.ZF == 0)
		OPERAND_W(op_dest, 1);
	else
		OPERAND_W(op_dest, 0);
	print_asm_template1();
}

#define instr sete
make_instr_helper(r2rm)
#undef instr

#define instr setl
make_instr_helper(r2rm)
#undef instr

#define instr setge
make_instr_helper(r2rm)
#undef instr

#define instr setg
make_instr_helper(r2rm)
#undef instr

#define instr seta
make_instr_helper(r2rm)
#undef instr

#define instr setne
make_instr_helper(r2rm)
#undef instr

#define instr setle
make_instr_helper(r2rm)
#undef instr

#include "cpu/exec/template-end.h"
