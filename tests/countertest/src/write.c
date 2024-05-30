#include <am.h>
#include <csr.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include "priv.h"
#include "countertest.h"
#include "counter.h"
#include "write.h"

#define check_minstret_diff(diff) \
    do { \
        asm volatile ( \
            "csrr %0, minstret \n" \
            "j " #diff "f \n" \
            "9: nop \n" \
            "8: nop \n" \
            "7: nop \n" \
            "6: nop \n" \
            "5: nop \n" \
            "4: nop \n" \
            "3: nop \n" \
            "csrr %1, minstret \n" \
            : "=r" (minstret_start), "=r" (minstret_end) \
        ); \
        if (minstret_end - minstret_start != diff) { \
            printf(MSG_WARNING "minstret is not accurate.\n"); \
            printf("minstret_end - minstret_start should be %d, ", diff); \
            printf("but it is %d - %d = %d.\n", minstret_end, minstret_start, minstret_end - minstret_start); \
            /* error += 1; */ \
        } \
    } while (0)

void test_minstret() {
    // test minstret
    uint64_t minstret_start, minstret_end;

    check_minstret_diff(3);
    check_minstret_diff(4);
    check_minstret_diff(5);
    check_minstret_diff(6);
    check_minstret_diff(7);
    check_minstret_diff(8);
    check_minstret_diff(9);

}


void test_counter_write() {
    goto_priv_mode(MODE_M);

    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_write_machine_counter[i]) {
            arr_func_write_machine_counter[i](0xFFFFFFFF);
        }
    }

    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_read_machine_counter[i]) {
            uint64_t cur = arr_func_read_machine_counter[i]();
            if (cur < 0xFFFFFFFF) {
                printf(MSG_WARNING "worte 0xFFFFFFFF to counter %d, ", i);
                printf("but read 0x%X.\n", cur);
            }
        }
    }

    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_write_machine_counter[i]) {
            arr_func_write_machine_counter[i](0);
        }
    }

    for (int i = 0; i < COUNTER_NUM; i++) {
        if (arr_func_read_machine_counter[i]) {
            uint64_t cur = arr_func_read_machine_counter[i]();
            if (cur >= 0xFFFFFFFF) {
                printf(MSG_WARNING "worte 0x0 to counter %d, ", i);
                printf("but read 0x%X.\n", cur);
            }
        }
    }
}
