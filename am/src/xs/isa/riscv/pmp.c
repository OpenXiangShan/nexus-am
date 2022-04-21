#include <xs.h>
#include <pmp.h>
#include <klib.h>
/*
 * Note that PMP should only be set on machine mode
 */

// Hence, when the PMP settings are modified, M-mode software must synchronize the PMP settings with the virtual memory system and any PMP or address-translation caches. This is accomplished by executing an SFENCE.VMA instruction with rs1=x0 and rs2=x0, after the PMP CSRs are written.


void init_pmp(bool restrict_machine_access) {
  // set all addr registers to disable possible access fault
  for (int i = 0; i < PMP_COUNT; i++) {
    csr_write_num(PMPADDR_BASE + i, -1L);
  }
  asm volatile("sfence.vma");
}

void enable_pmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t lock, uint8_t permission) {
  // by default using NAPOT
  assert((pmp_size & (pmp_size - 1)) == 0); // must be power of 2
  assert(pmp_reg < PMP_COUNT);
  if (pmp_size != 4) {
    // adjust pmp addr according to pmp size
    uintptr_t append = (pmp_size >> 3) - 1;
    pmp_addr |= append;
  }
  csr_write_num(PMPADDR_BASE + pmp_reg, pmp_addr);

  uint8_t set_content = permission | (lock << 7) | (pmp_size == 4 ? 2 : 3) << 3;
  uint16_t cfg_offset = pmp_reg > 7 ? 2 : 0;
  uint16_t cfg_shift = pmp_reg & 0x7;
  csr_set_num(PMPCFG_BASE + cfg_offset, set_content << cfg_shift);
  asm volatile("sfence.vma");
}

void enable_pmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uint8_t permission) {
    // similar interface but different implementation
    // pmp reg and pmp reg + 1 will be used
    assert(pmp_reg < PMP_COUNT);
    csr_write_num(PMPADDR_BASE + pmp_reg, pmp_addr + pmp_size);
    if (pmp_reg) {
      csr_write_num(PMPADDR_BASE + pmp_reg - 1, pmp_addr);
    }
    
    uint8_t set_content = permission | (lock << 7) | (1 << 3);
    uint16_t cfg_offset = pmp_reg > 7 ? 2 : 0;
    uint16_t cfg_shift = pmp_reg & 0x7;
    csr_set_num(PMPCFG_BASE + cfg_offset, set_content << cfg_shift);
    asm volatile("sfence.vma");
}

void disable_pmp(uintptr_t pmp_reg) {
  // just set pmp addr to max
  csr_write_num(PMPADDR_BASE + pmp_reg, -1L);
  asm volatile("sfence.vma");
}