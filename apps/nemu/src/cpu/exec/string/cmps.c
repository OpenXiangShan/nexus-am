#include "cpu/exec/helper.h"

make_helper(cmpsb) {
	uint8_t p_esi = swaddr_read(cpu.esi,1, R_ES);
	uint8_t p_edi = swaddr_read(cpu.edi,1, R_ES);
	uint8_t result = p_esi - p_edi;
	/* EFLAGS */
	uint8_t least = result & 0xFF;
	least ^= least>>4;
	least ^= least>>2;
	least ^= least>>1;
	bool test_PF = !(least & 0x1);
	test_PF?(cpu.EFLAGS.PF = 1):(cpu.EFLAGS.PF = 0);
	if(result == 0)
		cpu.EFLAGS.ZF = 1;
	else
		cpu.EFLAGS.ZF = 0;
	if(p_edi < p_esi)
		cpu.EFLAGS.CF = 1;
	else
		cpu.EFLAGS.CF = 0;

	if((int8_t)result > 0)
		cpu.EFLAGS.SF = 0;
	else
		cpu.EFLAGS.SF = 1;
	if(((int8_t)p_esi < 0 &&(int8_t)p_edi > 0 && (int8_t)result < 0) ||((int8_t)p_esi > 0 && (int8_t)p_edi < 0 && (int8_t)result > 0))
		cpu.EFLAGS.OF = 1;
	else
		cpu.EFLAGS.OF = 0;
	/* EFLAGS END */

	if(cpu.EFLAGS.DF == 0)
	{
		cpu.esi++;
		cpu.edi++;
	}
	else
	{
		cpu.esi--;
		cpu.edi--;
	}

	print_asm("cmpsb");
	return 2;
}
#include "cpu/exec/template-end.h"
