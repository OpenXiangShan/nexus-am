#include "cpu/exec/template-start.h"

#define instr in

static void do_execute() {
	assert(0);
	/*DATA_TYPE io_temp = pio_read(op_src->val, DATA_BYTE);
	if(DATA_BYTE == 1)
		cpu.eax = (cpu.eax & 0xFFFFFF00) + io_temp;
	else if(DATA_BYTE == 2)
		cpu.eax = (cpu.eax & 0xFFFF0000) + io_temp;
	else
		cpu.eax = io_temp;
	print_asm_template1();*/
}

make_instr_helper(i2a);

make_helper(concat(in_d2a_,SUFFIX)) {
	uint16_t dx = cpu.edx;
	DATA_TYPE io_temp = pio_read(dx, DATA_BYTE);
	if(DATA_BYTE == 1)
		REG(R_AL) = io_temp;
	else if(DATA_BYTE == 2)
		REG(R_AX) = io_temp;
	else
		REG(R_EAX) = io_temp;
	print_asm_template1();
	return 1;
}

#include "cpu/exec/template-end.h"
