
#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"

extern void m_trap_entry();

int main() {
    // setup
    csr_write(mtvec, m_trap_entry);

    goto_priv_mode(MODE_M);
    csr_write(mcounteren, 0);
    csr_write(scounteren, 0);
    csr_write(hcounteren, 0);

    goto_priv_mode(MODE_VU);

    last_exception.expected = 1;
    last_exception.cause = 0;
    last_exception.epc = 0;

    int this_cycle = csr_read(cycle);
    printf("cycle: %d\n", this_cycle);
    
    if (last_exception.cause == 2) {
        printf("good exception!\n");
        _halt(0);
    } else {
        printf("bad no exception!\n");
        _halt(1);
    }

    return 0;
}