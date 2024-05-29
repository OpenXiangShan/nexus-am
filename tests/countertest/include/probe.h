#ifndef __PROBE_H__
#define __PROBE_H__

typedef int (*func_probe_t)();
extern func_probe_t arr_func_probe_unpriv_cntr[COUNTER_NUM];
extern func_probe_t arr_func_probe_machine_cntr[COUNTER_NUM];

#define def_func_probe_csr(csr, addr) \
    int probe_counter_##csr() { \
        setup_expected_exception(); \
        int csr_read_res; \
        csr_read_res = csr_read(csr); \
        csr_read_res &= csr_read_res; \
        if (last_exception.actual_trap) { /* Not exist */ \
            printf(MSG_WARNING "" #csr " does not exist.\n"); \
            clear_last_exception(); \
            return -1; \
        } else { \
            clear_last_exception(); \
            return 0;\
        } \
    }

#define list_func_probe_csr(csr, addr) \
    probe_counter_##csr,

#endif // __PROBE_H__
