#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "push-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "push-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "push-template.h"
#undef DATA_BYTE

make_helper_v(push_r)
make_helper_v(push_rm)
make_helper_v(push_i)

make_helper(pusha) {
	uint32_t temp = cpu.esp;
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.eax, R_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.ecx, R_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.edx, R_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.ebx, R_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, temp, R_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.ebp, R_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.esi, R_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.edi, R_SS);
	print_asm("pusha");
	return 1;
}
