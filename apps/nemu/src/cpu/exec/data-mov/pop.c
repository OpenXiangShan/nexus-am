#include "cpu/exec/helper.h"

#define DATA_BYTE 2
#include "pop-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "pop-template.h"
#undef DATA_BYTE

make_helper_v(pop_r)
make_helper_v(pop_rm)

make_helper(popa) {
	cpu.edi = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp += 4;
	cpu.esi = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp += 4;
	cpu.ebp = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp += 4;
	/* throwaway esp */
	cpu.esp += 4;
	cpu.ebx = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp += 4;
	cpu.edx = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp += 4;
	cpu.ecx = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp += 4;
	cpu.eax = swaddr_read(cpu.esp, 4, R_SS);
	cpu.esp += 4;
	print_asm("popa");
	return 1;
}

