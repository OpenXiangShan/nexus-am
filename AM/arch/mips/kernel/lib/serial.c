#include "mips.h"
#include "device.h"

void init_serial(){
}

char *csend = SERIAL_PORT + Tx;
char *crecv = SERIAL_PORT + Rx;
char *stat = SERIAL_PORT + STAT;
char *ctrl = SERIAL_PORT + CTRL;

void out_byte(char ch){
	while(((*stat >> 3) & 0x1));
	*csend = ch;
}

char in_byte(){
	while(!(*stat & 0x1));
	return *crecv;
}

void
putchar(char ch) {
	out_byte(ch);
}
