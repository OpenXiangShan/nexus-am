#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "enable.h"

// define all check_enable_* function
MAP(CSRS_UNPRIV_COUNTER_TIMERS, def_func_check_enable)

// link all check_enable_* function to array of function pointers
func_enable_t check_enable_func_arr[32] = {
    MAP(CSRS_UNPRIV_COUNTER_TIMERS, list_func_check_enable)
};
