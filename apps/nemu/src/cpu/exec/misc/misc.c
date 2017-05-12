#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"
#include "x86-inc/mmu.h"
#include "x86-inc/addr_struct.h"

make_helper(nop) {
	print_asm("nop");
	return 1;
}

make_helper(int3) {
	void do_int3();
	do_int3();
	print_asm("int3");

	return 1;
}

make_helper(lea) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	int len = load_addr(eip + 1, &m, op_src);
	reg_l(m.reg) = op_src->addr;

	print_asm("leal %s,%%%s", op_src->str, regsl[m.reg]);
	return 1 + len;
}

make_helper(cld) {
	cpu.EFLAGS.DF = 0;
	print_asm("cld");
	return 1;
}

make_helper(std) {
	cpu.EFLAGS.DF = 1;
	print_asm("std");
	return 1;
}

make_helper(cli) {
	cpu.EFLAGS.IF = 0;
	print_asm("cli");
	return 1;
}

make_helper(sti) {
	cpu.EFLAGS.IF = 1;
	print_asm("sti");
	return 1;
}

make_helper(iret) {
	if(cpu.cr0.protect_enable == 0)
	{
		assert(0);
		/*cpu.eip = (cpu.eip & 0xFFFF0000) + swaddr_read(cpu.esp, 2, R_CS);
		cpu.esp += 2;
		cpu.CS.val = swaddr_read(cpu.esp, 2, R_CS);
		cpu.esp += 2;
		cpu.EFLAGS.val = (cpu.EFLAGS.val & 0xFFFF0000) + swaddr_read(cpu.esp, 2, R_CS);
		cpu.esp += 2;
		assert("HIT NOT GOOD");*/
	}
	else
	{
		cpu.eip = swaddr_read(cpu.esp, 4, R_CS) - 1;
		cpu.esp += 4;
		cpu.CS.val = swaddr_read(cpu.esp, 2, R_CS);
		uint16_t index = 0;
		index = cpu.CS.INDEX;
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
		cpu.CS.base = b.val;
		cpu.CS.limit = lim.val;
		cpu.CS.valid = true;
		cpu.esp += 4;
		cpu.EFLAGS.val = swaddr_read(cpu.esp, 4, R_SS);
		cpu.esp += 4;
	}
	print_asm("iret");
	return 1;
}

make_helper(hlt) {
	while(cpu.INTR == 0);
	//while(!(cpu.INTR & cpu.EFLAGS.IF));
	print_asm("hlt");
	return 1;
}

make_helper(lahf) {
	union reg {
		uint8_t r_ah;
		struct {
			uint8_t sf:1;
			uint8_t zf:1;
			uint8_t a:1;
			uint8_t af:1;
			uint8_t b:1;
			uint8_t pf:1;
			uint8_t c:1;
			uint8_t cf:1;
		};
	}reg_ah;
	reg_ah.sf = cpu.EFLAGS.SF;
	reg_ah.zf = cpu.EFLAGS.ZF;
	reg_ah.pf = cpu.EFLAGS.PF;
	reg_ah.cf = cpu.EFLAGS.CF;
	reg_ah.a = 0;
	reg_ah.af = 0;
	reg_ah.b = 0;
	reg_ah.c = 0;
	cpu.gpr[0]._8[1] = reg_ah.r_ah;
	return 1;
}
