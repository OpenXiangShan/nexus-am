#include "cpu/exec/template-start.h"

#define instr jcc

make_helper(concat(jz_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.ZF == 1)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(js_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.SF == 1)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jns_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.SF == 0)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jne_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.ZF == 0)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jnb_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.CF == 0)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jl_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.SF != cpu.EFLAGS.OF)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jle_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.ZF == 1 || cpu.EFLAGS.SF != cpu.EFLAGS.OF)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jg_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.ZF == 0 && cpu.EFLAGS.SF == cpu.EFLAGS.OF)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jge_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.SF == cpu.EFLAGS.OF)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(je_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.ZF == 1)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jc_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.CF == 1)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jbe_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.ZF == 1||cpu.EFLAGS.CF == 1)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(jb_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.CF == 1)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

make_helper(concat(ja_rel_,SUFFIX)){
	DATA_TYPE_S addr = instr_fetch(eip + 1 , DATA_BYTE);
	if(cpu.EFLAGS.CF == 0 && cpu.EFLAGS.ZF == 0)
		cpu.eip = cpu.eip + addr;
	print_asm(str(instr) " %x",cpu.eip+DATA_BYTE+1);
	return DATA_BYTE+1;
}

#include "cpu/exec/template-end.h"
