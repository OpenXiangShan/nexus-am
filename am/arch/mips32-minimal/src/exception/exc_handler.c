
#include <am.h>
#include <klib.h>


uint get_cause() {
	uint ret;
	asm volatile("mfc0 %0, $13\n\t":"=r"(ret));
	return ret;
}

uint get_epc() {
	uint ret;
	asm volatile("mfc0 %0, $14\n\t":"=r"(ret));
	return ret;
}

void exception_handle() {
	uint cause = get_cause();
	uint epc = get_epc();

	uint exccode = (cause >> 2) & 0x1f;

	switch(exccode) {
		case 4:		printf("BadVAddr when loading @PC= 0x%x", epc);		break;
		case 5:		printf("BadVAddr when storing @PC= 0x%x", epc);		break;
		case 10:	printf("Invalid OPCODE @PC= 0x%x", epc);	break;
		default:	printf("Unhandled exception, ExcCode = %d", exccode);
	}

	_halt(1);
}
