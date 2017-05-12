#include "cpu/exec/template-start.h"

#define instr mov

static void do_execute() {
	OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
}

make_instr_helper(i2r)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

make_helper(concat(mov_a2moffs_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	MEM_W(addr, REG(R_EAX), R_DS);
	print_asm("mov" str(SUFFIX) " %%%s,0x%x", REG_NAME(R_EAX), addr);
	return 5;
}

make_helper(concat(mov_moffs2a_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	REG(R_EAX) = MEM_R(addr, R_DS);
	print_asm("mov" str(SUFFIX) " 0x%x,%%%s", addr, REG_NAME(R_EAX));
	return 5;
}

make_helper(concat(mov_cr2r_,SUFFIX)) {
	uint8_t rm = instr_fetch(eip + 1,1) & 0x7;
	uint8_t reg = (instr_fetch(eip + 1,1) >> 3) & 0x7;
	if(reg == 0)
		REG(rm) = cpu.cr0.val;
	else if(reg == 3)
		REG(rm) = cpu.cr3.val;
	else
		assert(0);
	print_asm("mov" str(SUFFIX) " %s%x,%%%s", "CR", reg,  REG_NAME(rm));
	return 2;
}

make_helper(concat(mov_r2cr_,SUFFIX)) {
	uint8_t rm = instr_fetch(eip + 1,1) & 0x7;
	uint8_t reg = (instr_fetch(eip + 1,1) >> 3) & 0x7;
	if(reg == 0)
		cpu.cr0.val = REG(rm);
	else if(reg == 3)
		cpu.cr3.val = REG(rm);
	else
		assert(0);
	print_asm("mov" str(SUFFIX) " %%%s,%s%x", REG_NAME(rm),"CR",reg);
	return 2;
}

make_helper(concat(mov_a2sreg_,SUFFIX)) {
	uint8_t reg = (instr_fetch(eip + 1, 1) >> 3) & 0x7;
	uint16_t index = 0;
	switch(reg)
	{
		case R_ES:{
				cpu.ES.val = REG(R_EAX);
				index = cpu.ES.INDEX;
				break;
			}
		case R_CS:{
				cpu.CS.val = REG(R_EAX);
				index = cpu.CS.INDEX;
				break;
			}
		case R_SS:{
				cpu.SS.val = REG(R_EAX);
				index = cpu.SS.INDEX;
				break;
			}
		case R_DS:{
				cpu.DS.val = REG(R_EAX);
				index = cpu.DS.INDEX;
				break;
			}
	}
	sreg_desc s;
	base_addr b;
	limit lim;
	s.val[0] = lnaddr_read(cpu.GDTR.GDT_BASE + index * 8, 4);
	s.val[1] = lnaddr_read(cpu.GDTR.GDT_BASE + index * 8 + 4, 4);
	b.base15 = s.sr_des.base_15_0;
	b.base23 = s.sr_des.base_23_16;
	b.base31 = s.sr_des.base_31_24;
	lim.limit15 = s.sr_des.limit_15_0;
	lim.limit19 = s.sr_des.limit_19_16;
	
	switch(reg)
	{
		case R_ES:{
				cpu.ES.base = b.val;
				cpu.ES.limit = lim.val;
				cpu.ES.valid = true;
				break;
			}
		case R_CS:{
				cpu.CS.base = b.val;
				cpu.CS.limit = lim.val;
				cpu.CS.valid = true;
				break;
			}
		case R_SS:{
				cpu.SS.base = b.val;
				cpu.SS.limit = lim.val;
				cpu.SS.valid = true;
				break;
			}
		case R_DS:{
				cpu.DS.base = b.val;
				cpu.DS.limit = lim.val;
				cpu.DS.valid = true;
				break;
			}
	}
	print_asm("mov" str(SUFFIX) " %%%s,%s", REG_NAME(R_EAX), "sreg");
	return 2;
}

#include "cpu/exec/template-end.h"
