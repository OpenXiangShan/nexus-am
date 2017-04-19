#ifndef __ARCH_LIB__
#define __ARCH_LIB__

// These should not appear in the arch.h
// See AM spec for more details.

#define VMEM ((char *)0xc0000000)
#define KEY_CODE_ADDR ((volatile unsigned int *)0xf0000000)
#define KEY_CODE (*KEY_CODE_ADDR)
#define SCR_WIDTH 640
#define SCR_HEIGHT 480
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)

#endif
