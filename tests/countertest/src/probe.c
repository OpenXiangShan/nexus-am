#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "counter.h"
#include "probe.h"

// define all probe_counter_* function
MAP(CSRS_UNPRIV_COUNTER_TIMERS, def_func_probe_csr)
MAP(CSRS_M_COUNTER_TIMERS, def_func_probe_csr)

// link all probe_counter_* function to array of function pointers
func_probe_t arr_func_probe_unpriv_cntr[COUNTER_NUM] = {
    MAP(CSRS_UNPRIV_COUNTER_TIMERS, list_func_probe_csr)
};
func_probe_t arr_func_probe_machine_cntr[COUNTER_NUM] = {
    probe_counter_mcycle,
    NULL,
    probe_counter_minstret,
    MAP(CSRS_M_HPM, list_func_probe_csr)
};
