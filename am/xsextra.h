/*
 * Supplementary driver APIs for XiangShan test suit
 */

#ifndef __XSEXTRA_H__
#define __XSEXTRA_H__

#include <stdint.h>
#include <stddef.h>
#include <klib-macros.h>
#include ARCH_H // "arch/x86-qemu.h", "arch/native.h", ...

#ifdef __cplusplus
extern "C" {
#endif

// =============== Supplement L1Cache ===============

uintptr_t _l1cache_op_read(int op_idx);
uintptr_t _l1cache_data_read(int data_idx);
void _l1cache_op_write(int op_idx, uintptr_t val);
void _l1cache_data_write(int data_idx, uintptr_t val);

// ============== Supplement L2/3Cache ==============

void _l3cache_tag(int tag);
void _l3cache_set(int set);
void _l3cache_cmd(int cmd);

// ================= Supplement PMA =================
uintptr_t _pma_get_addr(int addr_idx);
uintptr_t _pma_get_cfg(int cfg_idx);
void _pma_set_addr(int addr_idx, uintptr_t val);
void _pma_set_cfg(int cfg_idx, uintptr_t val);

// ================= Supplement MPE =================
void _mpe_setncpu(char arg);
void _mpe_wakeup(int cpu);
intptr_t _atomic_add(volatile intptr_t *addr, intptr_t adder);
void _barrier();

// ================== PLIC driver ===================
uint32_t plic_get_claim(uint32_t current_context);
void plic_clear_intr(uint32_t claim);
void plic_clear_claim(uint32_t current_context, uint32_t claim);
void plic_set_priority(uint32_t intr, uint32_t priority);
void plic_enable(uint32_t current_context, uint32_t intr);
void plic_disable(uint32_t current_context, uint32_t intr);
void plic_disable_word(uint32_t current_context, uint32_t intr);
void plic_set_threshold(uint32_t current_context, uint32_t threshold);
void plic_set_intr(uint32_t intr);

// =================== PMP driver ===================
void init_pmp();
void enable_pmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t lock, uint8_t permission);
void enable_pmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uint8_t permission);
void disable_pmp(uintptr_t pmp_reg);

// =================== Timer driver =================
void init_timer();
void enable_timer();
void set_timer_inc(uintptr_t inc);

// =========== Interrupt handler registration =======
void ssip_handler_reg(_Context*(*handler)(_Event, _Context*));
void stip_handler_reg(_Context*(*handler)(_Event, _Context*));
void seip_handler_reg(_Context*(*handler)(_Event, _Context*));
void secall_handler_reg(_Context*(*handler)(_Event, _Context*));
void custom_handler_reg(uintptr_t code, _Context*(*handler)(_Event, _Context*));
void irq_handler_reg(uintptr_t code, _Context*(*handler)(_Event*, _Context*));

#ifdef __cplusplus
}
#endif

#endif