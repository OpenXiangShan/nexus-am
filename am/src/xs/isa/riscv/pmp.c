#include <xs.h>
#include <pmp.h>
// Hence, when the PMP settings are modified, M-mode software must synchronize the PMP settings with the virtual memory system and any PMP or address-translation caches. This is accomplished by executing an SFENCE.VMA instruction with rs1=x0 and rs2=x0, after the PMP CSRs are written.

void init_pmp(bool restrict_machine_access) {
    // clear all registers
}

void enable_pmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uintptr_t permission) {
    // by default using NAPOT
}

void enable_pmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uintptr_t permission) {
    // similar interface but different implementation
}
