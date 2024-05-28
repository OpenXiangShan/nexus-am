
#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "enable.h"

extern void m_trap_entry();

int error = 0;

int main() {
    // setup trap vector
    csr_write(mtvec, m_trap_entry);

    check_enable_func_arr[1] = 0;

    for (int i = 0; i < COUNTER_NUM; i++) {
        if (check_enable_func_arr[i]) {
            check_enable_func_arr[i]();
        }
    }

    return error;
}
