
#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "counter.h"
#include "enable.h"
#include "probe.h"
#include "inhibit.h"
#include "write.h"

extern void m_trap_entry();
int error = 0;

int main() {
    // setup trap vector
    csr_write(mtvec, m_trap_entry);

    // setup pmp to allow full access to physical memory in other modes
    csr_write(pmpcfg0, 0xf);
    csr_write(pmpaddr0, (uint64_t) -1);

    // probe machine counter
    printf("Probing machine counter...\n");
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_probe_machine_cntr[i]) {
            int res = arr_func_probe_machine_cntr[i]();
            if (res) {
                arr_func_probe_unpriv_cntr[i] = NULL;
                arr_func_check_enable[i] = NULL;
                arr_func_read_machine_counter[i] = NULL;
                arr_func_write_machine_counter[i] = NULL;
                error += 1;
            }
        }
    }

    // probe unprivileged counter
    printf("Probing unprivileged counter...\n");
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_probe_unpriv_cntr[i]) {
            int res = arr_func_probe_unpriv_cntr[i]();
            if (res) {
                arr_func_check_enable[i] = NULL;
                arr_func_read_unpriv_counter[i] = NULL;
            }
        }
    }

    // test counter-enable
    printf("Testing counter-enable...\n");
    test_counter_enable();

    // test counter-inhibit
    printf("Testing counter-inhibit...\n");
    test_counter_inhibit();

    // test writing to counter
    printf("Testing writing to counter...\n");
    test_counter_write();

    // test minstret
    printf("Testing minstret...\n");
    test_minstret();

    printf("Finished!\n");

    return error;
}
