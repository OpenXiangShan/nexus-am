
#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"

exception_t last_exception;

static inline uint64_t next_pc(uint64_t this_pc) {
    uint16_t this_instruction = * (uint16_t *) this_pc;
    if((this_instruction & 0b11) == 0b11) {
        return this_pc + 4;
    } else {
        return this_pc + 2;
    }
}

// C Exception handler
volatile int m_exception_handler(
    uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
    uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7
) {
    last_exception.cause = csr_read(mcause);
    last_exception.epc = csr_read(mepc);

    // handle ecall
    if (last_exception.cause >= 8 && last_exception.cause <= 11) {
        return ecall_handler(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    // other exceptions should be expected by main program
    assert(last_exception.expected_trap);
    last_exception.actual_trap = 1;

    // epc to next
    uint64_t this_mepc = csr_read(mepc);
    this_mepc = next_pc(this_mepc);
    csr_write(mepc, this_mepc);
    return 0;
}

// handle any ecall
volatile int ecall_handler(
    uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
    uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t ecall_num
) {
    // Reserve for processing more ecalls
    if (ecall_num == ECALL_GOTO_PRIV_MODE) {
        return ecall_goto_priv_mode_handler(arg0);
    } else {
        assert(0);
    }
}

// handle ecall for switch privileged mode
int ecall_goto_priv_mode_handler(uint64_t next_mode) {
    // set mstatus previous privilege mode / virtualization mode
    mstatus_t this_mstatus;
    this_mstatus.val = csr_read(mstatus);
    this_mstatus.mpp = next_mode & 0b11;
    this_mstatus.mpv = (next_mode >> 2) & 0b1;
    csr_write(mstatus, this_mstatus.val);

    // set epc to next
    uint64_t this_mepc = csr_read(mepc);
    this_mepc = next_pc(this_mepc);
    csr_write(mepc, this_mepc);
    return 0;
}

// Go to specific privilege mode by ecall
void goto_priv_mode(uint64_t next_mode) {
    do_ecall(next_mode, 0, 0, 0, 0, 0, 0, ECALL_GOTO_PRIV_MODE);
    // Regardless of the target mode, it will always first trap to M-mode using
    // an ecall. This approach simplifies the logic for mode switching.
}

// Do ecall from main program flow
void do_ecall(
    uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
    uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t ecall_num
) {
    // need this, or the compiler will compeletely ignore the parameters.
    register uint64_t a0 asm ("a0") = arg0;
    register uint64_t a1 asm ("a1") = arg1;
    register uint64_t a2 asm ("a2") = arg2;
    register uint64_t a3 asm ("a3") = arg3;
    register uint64_t a4 asm ("a4") = arg4;
    register uint64_t a5 asm ("a5") = arg5;
    register uint64_t a6 asm ("a6") = arg6;
    register uint64_t a7 asm ("a7") = ecall_num;

    asm volatile ("ecall"
                  : "+r" (a0), "+r" (a1), "+r" (a2), "+r" (a3), 
                    "+r" (a4), "+r" (a5), "+r" (a6), "+r" (a7));
}

void setup_expected_exception() {
    last_exception.expected_trap = 1;
    last_exception.actual_trap = 0;
    last_exception.cause = 0;
    last_exception.epc = 0;
}

void clear_last_exception() {
    last_exception.expected_trap = 0;
    last_exception.actual_trap = 0;
    last_exception.cause = 0;
    last_exception.epc = 0;
}
