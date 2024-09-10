#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "counter.h"
#include "inhibit.h"

static uint64_t saved_machine_counters[COUNTER_NUM];
static int could_not_be_inhibit[COUNTER_NUM] = {};

void test_counter_inhibit() {
    // test in M mode
    goto_priv_mode(MODE_M);

    // try to set mcountinhibit
    setup_expected_exception();
    csr_write(mcountinhibit, 0xffffffff);
    if (last_exception.actual_trap) {
        printf(MSG_WARNING "mcountinhibit does not exist. %d\n", last_exception.cause);
        return;
    }
    clear_last_exception();

    uint64_t this_mcountinhibit = csr_read(mcountinhibit);
    for (int i = 0; i < COUNTER_NUM; i++) {
        uint64_t this_bit = 1ULL << i;
        if (i == 1) { // mtime
            if (this_bit & this_mcountinhibit) {
                printf(MSG_ERROR "mtime should not be able to be inhibited.\n");
                error += 1;
            }
        } else if (!(this_bit & this_mcountinhibit)) {
            printf(MSG_WARNING "counter %d could not be inhibit.\n", i);
            could_not_be_inhibit[i] = 1;
        }
    }

    // save all machine counters
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_read_machine_counter[i]) {
            saved_machine_counters[i] = arr_func_read_machine_counter[i]();
        }
    }

    // check all machine counters
    for (int i = 0; i < COUNTER_NUM; i ++) {
        if (could_not_be_inhibit[i]) {
            continue;
        } else if (arr_func_read_machine_counter[i]) {
            uint64_t cur = arr_func_read_machine_counter[i]();
            if (cur != saved_machine_counters[i]) {
                printf(MSG_ERROR "machine counter %d changed after inhibiting.\n", i);
                error += 1;
            }
        }
    }

    // check all unprivileged counters
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (could_not_be_inhibit[i]) {
            continue;
        } else if (arr_func_read_machine_counter[i] && arr_func_read_unpriv_counter[i]) {
            uint64_t cur = arr_func_read_unpriv_counter[i]();
            if (cur != saved_machine_counters[i]) {
                printf(MSG_ERROR "unprivileged counter %d changed after inhibiting.\n", i);
                error += 1;
            }
        }
    }

    csr_write(mcountinhibit, 0);
}