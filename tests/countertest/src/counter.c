
#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "countertest.h"
#include "counter.h"

MAP(CSRS_M_COUNTER_TIMERS, def_func_csr_read)
MAP(CSRS_UNPRIV_COUNTER_TIMERS, def_func_csr_read)
MAP(CSRS_M_COUNTER_TIMERS, def_func_csr_write)

func_csr_read_t arr_func_read_machine_counter[COUNTER_NUM] = {
    csr_read_mcycle,
    NULL,
    csr_read_minstret,
    MAP(CSRS_M_HPM, list_func_csr_read)
};
func_csr_write_t arr_func_write_machine_counter[COUNTER_NUM] = {
    csr_write_mcycle,
    NULL,
    csr_write_minstret,
    MAP(CSRS_M_HPM, list_func_csr_write)
};
func_csr_read_t arr_func_read_unpriv_counter[COUNTER_NUM] = {
    MAP(CSRS_UNPRIV_COUNTER_TIMERS, list_func_csr_read)
};
