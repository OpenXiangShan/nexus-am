#ifndef __NEMU_H__
#define __NEMU_H__

#ifdef __ARCH_X86_NEMU
#include <x86.h>
#define KBD_ADDR     0x60
#define RTC_ADDR     0x48
#define SCREEN_ADDR  0x100
#define SYNC_ADDR    0x104
#define FB_ADDR      0x40000
#endif

#ifdef __ARCH_MIPS32_NEMU
#include <mips32.h>
#define KBD_ADDR     0x4060
#define RTC_ADDR     0x4048
#define SCREEN_ADDR  0x4100
#define SYNC_ADDR    0x4104
#define FB_ADDR      0xa0040000
#endif

#ifdef __ARCH_RISCV32_NEMU
#include <riscv32.h>
#define KBD_ADDR     0x4060
#define RTC_ADDR     0x4048
#define SCREEN_ADDR  0x4100
#define SYNC_ADDR    0x4104
#define FB_ADDR      0x40000
#endif

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096

#endif
