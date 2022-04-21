/*
 * Supplementary driver APIs for XiangShan test suit
 */

#ifndef __XSEXTRA_H__
#define __XSEXTRA_H__

#include <stdint.h>
#include <stddef.h>
#include ARCH_H // "arch/x86-qemu.h", "arch/native.h", ...

#ifdef __cplusplus
extern "C" {
#endif

// ================= Supplement MPE =================
void _mpe_setncpu(char arg);
void _mpe_wakeup(int cpu);
intptr_t _atomic_add(volatile intptr_t *addr, intptr_t adder);

#ifdef __cplusplus
}
#endif

#endif