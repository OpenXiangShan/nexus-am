#include "cpu/exec/template-start.h"

#define instr out

make_helper(concat(out_a2i_,SUFFIX)) {
	assert(0);
	/*DATA_TYPE io_temp = cpu.eax;
	op_dest->val = instr_fetch(cpu.eip + 1, DATA_BYTE);
	pio_write(op_dest->val,DATA_BYTE,io_temp);
	print_asm_template1();
	return DATA_BYTE + 1;*/
}

make_helper(concat(out_a2d_,SUFFIX)) {
	DATA_TYPE io_temp = cpu.eax;
	uint16_t dx = cpu.edx;
	pio_write(dx,DATA_BYTE,io_temp);
	print_asm("out" str(SUFFIX) " al,(dl)");
	return 1;
}

#include "cpu/exec/template-end.h"
