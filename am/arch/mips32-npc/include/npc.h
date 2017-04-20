#ifndef __NPC_H__
#define __NPC_H__

#define VMEM_ADDR ((char *)0xc0000000)
#define KEY_CODE_ADDR ((volatile unsigned int *)0xf0000000)
#define KEY_CODE (*KEY_CODE_ADDR)
#define SCR_WIDTH 640
#define SCR_HEIGHT 480
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)
char GetColor(_Pixel p){
}

void vga_init();

#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c
#define SERIAL_PORT ((char *)0xe0000000)

#endif
