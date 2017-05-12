#include <setjmp.h>
#include "x86-inc/mmu.h"
#include "x86-inc/addr_struct.h"
#include "cpu/reg.h"

extern jmp_buf jbuf;

static void raise_intr(uint8_t NO) {
	/* TODO: Trigger an interrupt/exception with "NO"
	 * That is, use "NO" to index the IDT.
	 */
	cpu.esp = cpu.esp - 4;
	swaddr_write(cpu.esp, 4, cpu.EFLAGS.val, R_SS);
	cpu.esp = cpu.esp - 4;
	swaddr_write(cpu.esp, 2, cpu.CS.val, R_CS);
	cpu.esp = cpu.esp - 4;		
	if(NO == 0x80)
		swaddr_write(cpu.esp, 4, cpu.eip+2, R_CS);
	else
		swaddr_write(cpu.esp, 4, cpu.eip, R_CS);
	gate_desc gate_temp;
	gate_temp.val[0] = lnaddr_read(cpu.IDTR.IDT_BASE + NO * 8,4);
	gate_temp.val[1] = lnaddr_read(cpu.IDTR.IDT_BASE + NO * 8 + 4,4);
	cpu.CS.val = gate_temp.gate.segment;
	sreg_desc seg_temp;
	seg_temp.val[0] = lnaddr_read(cpu.GDTR.GDT_BASE + cpu.CS.INDEX * 8,4);
	seg_temp.val[1] = lnaddr_read(cpu.GDTR.GDT_BASE + cpu.CS.INDEX * 8 + 4,4);
	base_addr base;
	limit lim;
	base.base15 = seg_temp.sr_des.base_15_0;
	base.base23 = seg_temp.sr_des.base_23_16;
	base.base31 = seg_temp.sr_des.base_31_24;
	lim.limit15 = seg_temp.sr_des.limit_15_0;
	lim.limit19 = seg_temp.sr_des.limit_19_16;
	cpu.CS.base = base.val;
	cpu.CS.limit = lim.val;
	cpu.CS.valid = true;
	offset gate_offset;
	gate_offset.offset15 = gate_temp.gate.offset_15_0;
	gate_offset.offset31 = gate_temp.gate.offset_31_16;
	cpu.eip = base.val + gate_offset.val;
	/* Jump back to cpu_exec() */
	longjmp(jbuf, 1);
}
