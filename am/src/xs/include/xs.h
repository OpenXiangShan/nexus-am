#ifndef __XIANGSHAN_H__
#define __XIANGSHAN_H__

#include <am.h>
#include <xsextra.h>
#include <klib-macros.h>
#include <klib.h>

#include ISA_H // "x86.h", "mips32.h", ...

extern int __am_ncpu;
// timer related driver functions
void init_timer();
void enable_timer();
void set_timer_inc(uintptr_t inc);

#endif

