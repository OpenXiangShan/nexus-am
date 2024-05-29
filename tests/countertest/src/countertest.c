
#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "enable.h"
#include "probe.h"

extern void m_trap_entry();

int error = 0;

int main() {
    // setup trap vector
    csr_write(mtvec, m_trap_entry);

    // probe machine counter
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (probe_machine_cntr_func_arr[i]) {
            int res = probe_machine_cntr_func_arr[i]();
            if (res) {
                probe_unpriv_cntr_func_arr[i] = NULL;
                check_enable_func_arr[i] = NULL;
                error += 1;
            }
        }
    }

    // probe unprivileged counter
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (probe_unpriv_cntr_func_arr[i]) {
            int res = probe_unpriv_cntr_func_arr[i]();
            if (res) {
                check_enable_func_arr[i] = NULL;
            }
        }
    }

    // test counter-enable
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (check_enable_func_arr[i]) {
            check_enable_func_arr[i]();
        }
    }

    return error;
}
