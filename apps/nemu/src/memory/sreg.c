#include "common.h"
#include "cpu/reg.h"
#include "x86-inc/mmu.h"
#include "x86-inc/addr_struct.h"

uint32_t lnaddr_read(lnaddr_t addr, size_t len);

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) {
	assert(sreg == 0 || sreg == 1 || sreg == 2 || sreg == 3);
	if(cpu.cr0.protect_enable == 0)
		return addr;
	uint8_t ti = 0;
	uint16_t index = 0;
	bool cache = false;
	switch(sreg)
	{
		case R_ES:{ti = cpu.ES.TI;
			index = cpu.ES.INDEX;
			cache = cpu.ES.valid;
break;}
		case R_CS:{ti = cpu.CS.TI;
			index = cpu.CS.INDEX;
			cache = cpu.CS.valid;
break;}
		case R_SS:{ti = cpu.SS.TI;
			index = cpu.SS.INDEX;
			cache = cpu.SS.valid;
break;}
		case R_DS:{ti = cpu.DS.TI;
			index = cpu.DS.INDEX;
			cache = cpu.DS.valid;
break;}
		default:assert("hit strange sreg!");
	}
	sreg_desc s;
	base_addr base;
	limit lim;
	if(ti == 0)
	{
		/* read from cache */
		if(cache == true)
		{
			switch(sreg)
			{
				case R_ES:base.val = cpu.ES.base;break;
				case R_CS:base.val = cpu.CS.base;break;
				case R_SS:base.val = cpu.SS.base;break;
				case R_DS:base.val = cpu.DS.base;break;
			}
			return addr + base.val;
		}
		/* read from GDT */
		s.val[0] = lnaddr_read(cpu.GDTR.GDT_BASE + index * 8, 4);
		s.val[1] = lnaddr_read(cpu.GDTR.GDT_BASE + index * 8 + 4, 4);
		base.base15 = s.sr_des.base_15_0;
		base.base23 = s.sr_des.base_23_16;
		base.base31 = s.sr_des.base_31_24;
		lim.limit15 = s.sr_des.limit_15_0;
		lim.limit19 = s.sr_des.limit_19_16;
		/* set cache */
		switch(sreg)
			{
				case R_ES:
				{
					cpu.ES.base = base.val;
					cpu.ES.limit = lim.val;
					cpu.ES.valid = true;
					break;
				}
				case R_CS:
				{
					cpu.CS.base = base.val;
					cpu.CS.limit = lim.val;
					cpu.CS.valid = true;
					break;
				}
				case R_DS:
				{
					cpu.DS.base = base.val;
					cpu.DS.limit = lim.val;
					cpu.DS.valid = true;
					break;
				}
				case R_SS:
				{
					cpu.SS.base = base.val;
					cpu.SS.limit = lim.val;
					cpu.SS.valid = true;
					break;
				}
			}
	}
	else
	{
		printf("cache = %d.sreg = %x\n",cache,sreg);
		Log("hit LDT");
		assert(0);
	}
	return addr + base.val;
}
