#include <xs.h>
#include <csr.h>
#include <klib.h>

// these functions are interface for pma
// uintptr_t _pma_get_addr(int addr_idx): return value in the pmpaddr csr
// uintptr_t _pma_get_cfg(int cfg_idx): return value in the pmpefg csr
// void _pma_set_addr(int addr_idx, uintptr_t val): modify value in the pmpaddr csr
// void _pma_set_cfg(int cfg_idx, uintptr_t val): modify value in the pmpcfg csr
// when testing pma:
// firstly,  using  _cfg = _pma_get_cfg(cfg_idx), get the value of cfg
// secondly, using _cfg = _cfg | PMP_W, or using _cfg = _cfg & ~PMP_W, to set the permission
// finally, using _pma_set_cfg(cfg_idx, _cfg), write the value into the pmpcfg csr

uintptr_t _pma_get_addr(int addr_idx) {
    uintptr_t addr;
    switch(addr_idx){
        case 0: asm volatile("csrr %0, pmpaddr0" : "=r"(addr)); break;
        case 1: asm volatile("csrr %0, pmpaddr1" : "=r"(addr)); break;
        case 2: asm volatile("csrr %0, pmpaddr2" : "=r"(addr)); break;
        case 3: asm volatile("csrr %0, pmpaddr3" : "=r"(addr)); break;
        case 4: asm volatile("csrr %0, pmpaddr4" : "=r"(addr)); break;
        case 5: asm volatile("csrr %0, pmpaddr5" : "=r"(addr)); break;
        case 6: asm volatile("csrr %0, pmpaddr6" : "=r"(addr)); break;
        case 7: asm volatile("csrr %0, pmpaddr7" : "=r"(addr)); break;
        case 8: asm volatile("csrr %0, pmpaddr8" : "=r"(addr)); break;
        case 9: asm volatile("csrr %0, pmpaddr9" : "=r"(addr)); break;
        case 10: asm volatile("csrr %0, pmpaddr10" : "=r"(addr)); break;
        case 11: asm volatile("csrr %0, pmpaddr11" : "=r"(addr)); break;
        case 12: asm volatile("csrr %0, pmpaddr12" : "=r"(addr)); break;
        case 13: asm volatile("csrr %0, pmpaddr13" : "=r"(addr)); break;
        case 14: asm volatile("csrr %0, pmpaddr14" : "=r"(addr)); break;
        case 15: asm volatile("csrr %0, pmpaddr15" : "=r"(addr)); break;
        default: addr = 0; break;
    }
    return addr;
}

uintptr_t _pma_get_cfg(int cfg_idx) {
    uintptr_t cfg;
    switch(cfg_idx){
        case 0: asm volatile("csrr %0, pmpcfg0" : "=r"(cfg)); break;
        case 1: asm volatile("csrr %0, pmpcfg1" : "=r"(cfg)); break;
        case 2: asm volatile("csrr %0, pmpcfg2" : "=r"(cfg)); break;
        case 3: asm volatile("csrr %0, pmpcfg3" : "=r"(cfg)); break;
        default: cfg = 0; break;
    }
    return cfg;
}

void _pma_set_addr(int addr_idx, uintptr_t val) {
    switch(addr_idx){
        case 0: asm volatile("csrw pmpaddr0, %0" : : "r"(val)); break;
        case 1: asm volatile("csrw pmpaddr1, %0" : : "r"(val)); break;
        case 2: asm volatile("csrw pmpaddr2, %0" : : "r"(val)); break;
        case 3: asm volatile("csrw pmpaddr3, %0" : : "r"(val)); break;
        case 4: asm volatile("csrw pmpaddr4, %0" : : "r"(val)); break;
        case 5: asm volatile("csrw pmpaddr5, %0" : : "r"(val)); break;
        case 6: asm volatile("csrw pmpaddr6, %0" : : "r"(val)); break;
        case 7: asm volatile("csrw pmpaddr7, %0" : : "r"(val)); break;
        case 8: asm volatile("csrw pmpaddr8, %0" : : "r"(val)); break;
        case 9: asm volatile("csrw pmpaddr9, %0" : : "r"(val)); break;
        case 10: asm volatile("csrw pmpaddr10, %0" : : "r"(val)); break;
        case 11: asm volatile("csrw pmpaddr11, %0" : : "r"(val)); break;
        case 12: asm volatile("csrw pmpaddr12, %0" : : "r"(val)); break;
        case 13: asm volatile("csrw pmpaddr13, %0" : : "r"(val)); break;
        case 14: asm volatile("csrw pmpaddr14, %0" : : "r"(val)); break;
        case 15: asm volatile("csrw pmpaddr15, %0" : : "r"(val)); break;
        default: break;
    }
}

void _pma_set_cfg(int cfg_idx, uintptr_t val) {
    switch(cfg_idx){
        case 0: asm volatile("csrw pmpcfg0, %0" : : "r"(val)); break;
        case 1: asm volatile("csrw pmpcfg1, %0" : : "r"(val)); break;
        case 2: asm volatile("csrw pmpcfg2, %0" : : "r"(val)); break;
        case 3: asm volatile("csrw pmpcfg3, %0" : : "r"(val)); break;
        default: break;
    }
}