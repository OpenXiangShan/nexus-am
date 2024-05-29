
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

extern void m_trap_entry();
int error = 0;

int main() {
    // setup trap vector
    csr_write(mtvec, m_trap_entry);

    // probe machine counter
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
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_check_enable[i]) {
            arr_func_check_enable[i]();
        }
    }

    // test counter-inhibit and writing to counter
    test_counter_inhibit();

    return error;
}
