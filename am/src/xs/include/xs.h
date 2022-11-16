#ifndef __XIANGSHAN_H__
#define __XIANGSHAN_H__

#include <am.h>
#include <xsextra.h>
#include <klib.h>

#include ISA_H // "x86.h", "mips32.h", ...

#define MAX_INTERNAL_INTR 10UL
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS) || defined(__ARCH_RISCV64_XS_FLASH)
#define MAX_EXTERNAL_INTR      64UL
#define INTR_GEN_ADDR          (0x40070000UL)
#define INTR_RANDOM            (INTR_GEN_ADDR + (MAX_EXTERNAL_INTR / 8))
#define INTR_RANDOM_MASK       (INTR_GEN_ADDR + (MAX_EXTERNAL_INTR / 8) * 2)
#define PLIC_BASE_ADDR         (0x3c000000UL)
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
#define MAX_EXTERNAL_INTR      256UL
#define INTR_GEN_ADDR          (0x1f00060000UL)
#define INTR_RANDOM            (INTR_GEN_ADDR + (MAX_EXTERNAL_INTR / 8))
#define INTR_RANDOM_MASK       (INTR_GEN_ADDR + (MAX_EXTERNAL_INTR / 8) * 2)
#define PLIC_BASE_ADDR         (0x1f1c000000UL)
#endif

extern int __am_ncpu;

#endif
