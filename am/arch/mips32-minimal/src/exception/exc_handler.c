#include <am.h>
#include <klib.h>

/*
void printstr(char *str) {
	while(*str ++) _putc(*str);
}
*/

unsigned int get_cause() {
	unsigned int ret;
	asm volatile("mfc0 %0, $13\n\t":"=r"(ret));
	return ret;
}

unsigned int get_epc() {
	unsigned int ret;
	asm volatile("mfc0 %0, $14\n\t":"=r"(ret));
	return ret;
}

void exception_handle() {
	unsigned int cause = get_cause();
	unsigned int epc = get_epc();

	unsigned int exccode = (cause >> 2) & 0x1f;

	switch(exccode) {
		case 4:		printf("BadVAddr when loading @PC = 0x%x.\n", epc);		break;
		case 5:		printf("BadVAddr when storing @PC = 0x%x.\n", epc);		break;
		case 10:	printf("Invalid OPCODE @PC = 0x%x.\n", epc);			break;
		default:	printf("Unhandled exception, ExcCode = %d.\n", exccode);
		/*
		case 4:		printstr("BadVAddr when loading.\n");		break;
		case 5:		printstr("BadVAddr when storing.\n");		break;
		case 10:	printstr("Invalid OPCODE.\n");				break;
		default:	printstr("Unhandled exception.\n");
		*/
	}

	_halt(1);
}
