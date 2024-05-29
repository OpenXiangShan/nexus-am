#ifndef __ENABLE_H__
#define __ENABLE_H__

typedef void (*func_enable_t)();
extern func_enable_t arr_func_check_enable[COUNTER_NUM];

#define check_enable_detail(csr, addr, m, h, s, mode, res) \
    do { \
        goto_priv_mode(MODE_M); \
        this_mcounteren = m << (addr - UNPRIV_COUNTER_BASE); \
        this_hcounteren = h << (addr - UNPRIV_COUNTER_BASE); \
        this_scounteren = s << (addr - UNPRIV_COUNTER_BASE); \
        csr_write(mcounteren, this_mcounteren); \
        csr_write(hcounteren, this_hcounteren); \
        csr_write(scounteren, this_scounteren); \
        goto_priv_mode(mode); \
        setup_expected_exception(); \
        csr_read_res = csr_read(csr); \
        csr_read_res &= csr_read_res; \
        if (res) { /* should trap */ \
            if (!last_exception.actual_trap) { \
                printf(MSG_ERROR "checking " #csr ", "); \
                printf("m/h/scounteren: %d/%d/%d, mode: " #mode "\n", m, h, s); \
                printf("reading " #csr " shoule trap into " #res ", but not.\n"); \
                error += 1; \
            } else if (last_exception.cause != res) { \
                printf(MSG_ERROR "checking " #csr ", "); \
                printf("m/h/scounteren: %d/%d/%d, mode: " #mode "\n", m, h, s); \
                printf("reading " #csr " shoule trap into " #res ", but trapped into %d.\n", last_exception.cause); \
                error += 1; \
            } \
        } else { /* should be accessible */\
            if (last_exception.actual_trap) { \
                printf(MSG_ERROR "checking " #csr ", "); \
                printf("m/h/scounteren: %d/%d/%d, mode: " #mode "\n", m, h, s); \
                printf(#csr " shoule be accessible, but not.\n"); \
                error += 1; \
            } \
        } \
        clear_last_exception(); \
    } while (0)

#define def_func_check_enable(csr, addr) \
    void check_enable_##csr() { \
        uint64_t this_mcounteren; \
        uint64_t this_hcounteren; \
        uint64_t this_scounteren; \
        uint64_t csr_read_res; \
        /* m/h/scounteren: 0/0/0 */ \
        check_enable_detail(csr, addr, 0, 0, 0, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 0, 0, 0, MODE_S,  EX_II); \
        check_enable_detail(csr, addr, 0, 0, 0, MODE_U,  EX_II); \
        check_enable_detail(csr, addr, 0, 0, 0, MODE_VS, EX_II); \
        check_enable_detail(csr, addr, 0, 0, 0, MODE_VU, EX_II); \
        /* m/h/scounteren: 0/0/1 */ \
        check_enable_detail(csr, addr, 0, 0, 1, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 0, 0, 1, MODE_S,  EX_II); \
        check_enable_detail(csr, addr, 0, 0, 1, MODE_U,  EX_II); \
        check_enable_detail(csr, addr, 0, 0, 1, MODE_VS, EX_II); \
        check_enable_detail(csr, addr, 0, 0, 1, MODE_VU, EX_II); \
        /* m/h/scounteren: 0/1/0 */ \
        check_enable_detail(csr, addr, 0, 1, 0, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 0, 1, 0, MODE_S,  EX_II); \
        check_enable_detail(csr, addr, 0, 1, 0, MODE_U,  EX_II); \
        check_enable_detail(csr, addr, 0, 1, 0, MODE_VS, EX_II); \
        check_enable_detail(csr, addr, 0, 1, 0, MODE_VU, EX_II); \
        /* m/h/scounteren: 0/1/1 */ \
        check_enable_detail(csr, addr, 0, 1, 1, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 0, 1, 1, MODE_S,  EX_II); \
        check_enable_detail(csr, addr, 0, 1, 1, MODE_U,  EX_II); \
        check_enable_detail(csr, addr, 0, 1, 1, MODE_VS, EX_II); \
        check_enable_detail(csr, addr, 0, 1, 1, MODE_VU, EX_II); \
        /* m/h/scounteren: 1/0/0 */ \
        check_enable_detail(csr, addr, 1, 0, 0, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 0, 0, MODE_S,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 0, 0, MODE_U,  EX_II); \
        check_enable_detail(csr, addr, 1, 0, 0, MODE_VS, EX_VI); \
        check_enable_detail(csr, addr, 1, 0, 0, MODE_VU, EX_VI); \
        /* m/h/scounteren: 1/0/1 */ \
        check_enable_detail(csr, addr, 1, 0, 1, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 0, 1, MODE_S,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 0, 1, MODE_U,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 0, 1, MODE_VS, EX_VI); \
        check_enable_detail(csr, addr, 1, 0, 1, MODE_VU, EX_VI); \
        /* m/h/scounteren: 1/1/0 */ \
        check_enable_detail(csr, addr, 1, 1, 0, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 1, 0, MODE_S,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 1, 0, MODE_U,  EX_II); \
        check_enable_detail(csr, addr, 1, 1, 0, MODE_VS, ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 1, 0, MODE_VU, EX_VI); \
        /* m/h/scounteren: 1/1/1 */ \
        check_enable_detail(csr, addr, 1, 1, 1, MODE_M,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 1, 1, MODE_S,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 1, 1, MODE_U,  ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 1, 1, MODE_VS, ACCESSIBLE); \
        check_enable_detail(csr, addr, 1, 1, 1, MODE_VU, ACCESSIBLE); \
    }

#define list_func_check_enable(csr, addr) \
    check_enable_##csr,

void test_counter_enable();

#endif // __ENABLE_H__
