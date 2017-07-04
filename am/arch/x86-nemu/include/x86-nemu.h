#ifndef __X86_NEMU_H__
#define __X86_NEMU_H__

/* Uncomment these macros to enable corresponding functionality. */
#define IA32_SEG
//#define IA32_PAGE
//#define IA32_INTR
#define HAS_DEVICE
//#define USE_HARDDISK

#ifndef __ASSEMBLER__
/* The following code will be included if the source file is a "*.c" file. */

#include <arch.h>

#endif

#endif
