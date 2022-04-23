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

// PMP related driver functions
void init_pmp();
void enable_pmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t lock, uint8_t permission);
void enable_pmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uint8_t permission);
void disable_pmp(uintptr_t pmp_reg);

// plic related driver functions
uint32_t plic_get_claim(uint32_t current_context);
void plic_clear_intr(uint32_t claim);
void plic_clear_claim(uint32_t current_context, uint32_t claim);
void plic_set_priority(uint32_t intr, uint32_t priority);
void plic_enable(uint32_t current_context, uint32_t intr);
void plic_disable_word(uint32_t current_context, uint32_t intr);
void plic_set_threshold(uint32_t current_context, uint32_t threshold);
void plic_set_intr(uint32_t intr);

#endif

