#ifndef __X86_NEMU_H__
#define __X86_NEMU_H__

//#define HAS_DEVICE

#ifndef __ASSEMBLER__

#include <arch.h>

#ifdef HAS_DEVICE
# define SERIAL_PORT ((volatile char *)0x3f8)
#endif

struct TrapFrame{
};

#endif

#endif
