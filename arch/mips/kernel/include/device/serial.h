#ifndef __SERIAL_H__
#define __SERIAL_H__

#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c
#define SERIAL_PORT ((char *)0xe0000000)

void init_serial();
void out_byte(char ch);
char in_byte();
void putchar(char ch);

#endif
