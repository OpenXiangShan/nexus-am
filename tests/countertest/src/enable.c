#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "counter.h"
#include "enable.h"

// define all check_enable_* function
MAP(CSRS_UNPRIV_COUNTER_TIMERS, def_func_check_enable)

// link all check_enable_* function to array of function pointers
func_enable_t arr_func_check_enable[COUNTER_NUM] = {
    MAP(CSRS_UNPRIV_COUNTER_TIMERS, list_func_check_enable)
};

void test_counter_enable() {
    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_check_enable[i]) {
            arr_func_check_enable[i]();
        }
    }
}
