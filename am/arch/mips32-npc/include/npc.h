#ifndef __NPC_H__
#define __NPC_H__

#define VMEM ((char *)0xc0000000)
#define KEY_CODE_ADDR ((volatile unsigned int *)0xf0000000)
#define KEY_CODE (*KEY_CODE_ADDR)
#define SCR_WIDTH 640
#define SCR_HEIGHT 480
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)

#endif
