#include <am.h>

static void printstr(char *str) {
	while(*str ++) _putc(*str);
}

static uint get_cause() {
	uint ret;
	asm volatile("mfc0 %0, $13\n\t":"=r"(ret));
	return ret;
}

static uint get_epc() {
	uint ret;
	asm volatile("mfc0 %0, $14\n\t":"=r"(ret));
	return ret;
}

void exception_handle() {
	uint cause = get_cause();
	//uint epc = get_epc();

	uint exccode = (cause >> 2) & 0x1f;

	switch(exccode) {
		/* printf verison
		case 4:		printf("BadVAddr when loading @PC = 0x%d.\n", epc);		break;
		case 5:		printf("BadVAddr when storing @PC = 0x%d.\n", epc);		break;
		case 10:	printf("Invalid OPCODE @PC = 0x%d.\n", epc);			break;
		default:	printf("Unhandled exception, ExcCode = %d.\n", exccode);
		*/
		case 4:		printstr("BadVAddr when loading.\n");		break;
		case 5:		printstr("BadVAddr when storing.\n");		break;
		case 10:	printstr("Invalid OPCODE.\n");				break;
		default:	printstr("Unhandled exception.\n");
	}

	_halt(1);
}
