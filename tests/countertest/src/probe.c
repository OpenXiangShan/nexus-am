#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "probe.h"

// define all probe_counter_* function
MAP(CSRS_UNPRIV_COUNTER_TIMERS, def_func_probe_csr)
MAP(CSRS_M_COUNTER_TIMERS, def_func_probe_csr)

// link all probe_counter_* function to array of function pointers
func_probe_t probe_unpriv_cntr_func_arr[32] = {
    MAP(CSRS_UNPRIV_COUNTER_TIMERS, list_func_probe_csr)
};
func_probe_t probe_machine_cntr_func_arr[32] = {
    probe_counter_mcycle,
    NULL,
    probe_counter_minstret,
    MAP(CSRS_M_HPM, list_func_probe_csr)
};
