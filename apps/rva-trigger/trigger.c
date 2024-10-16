/*
** It may be useless because it may be a wrong test invloved with the function call and asm inlined.
*/

#include <klib.h>
#include <stdint.h>
#define MCONTROL6_M          (1<<6)
#define MCONTROL6_LOAD       (1<<0)
#define MCONTROL6_STORE       (1<<1)
#define TRIGGER_LOAD  0
#define TRIGGER_STORE 1
#define TRIGGER_LOAD_AND_STORE 2
#define __riscv_xlen 64
#define TDATA1_TYPE 6ULL // mcontrol6
#define CAUSE_NONE 0
#define CAUSE_BP   3

void check_and_print(int r_cause){
    uint64_t mcause;
    asm volatile( "csrr %0, mcause\n\t"
                  "csrwi mcause, 0\n\t"
                  "csrwi tdata1, 0\n\t"
                  :"=r"(mcause):);
    if(r_cause == mcause)
        printf("\033[40;32;5m PASS: \033[0m");
    else
        printf("\033[40;31;5m FAIL: \033[0m");
    printf("mcause: %d\n", mcause);
}

void setTrigger(unsigned long tdata2, int op){
    asm volatile(
        "csrw tselect, 1\n\t"
        "csrw tdata2, %0\n\t"
        "csrs mstatus, 8\n\t"
        :
        : "r"(tdata2)
        :
    );
    if (op == 0){
        // load
        asm volatile(
            "csrw tdata1, %0\n\t"
            :
            : "r"((TDATA1_TYPE << (__riscv_xlen - 4)) | MCONTROL6_M | MCONTROL6_LOAD)
            :
        );
    } else if (op == 1){
        // store
        asm volatile(
            "csrw tdata1, %0\n\t"
            :
            : "r"((TDATA1_TYPE << (__riscv_xlen - 4)) | MCONTROL6_M | MCONTROL6_STORE)
            :
        );
    } else if (op == 2){
        // load and store
        asm volatile(
            "csrw tdata1, %0\n\t"
            :
            : "r"((TDATA1_TYPE << (__riscv_xlen - 4)) | MCONTROL6_M | MCONTROL6_LOAD | MCONTROL6_STORE)
            :
        );
    } else {
        printf(" Error: op should be 0 or 1\n");
    }
}

/*
** The value in the BASE field of mtvec 
** must always be aligned on a 4-byte boundary
** aligned(4) means aligned on a 4-byte boundary
** not aligned on a 2^4 byte boundary
*/
__attribute__((aligned(4))) void __am_asm_trap(void){
    asm volatile(
    "csrr t0, mepc\n\t"
    "addi t0, t0, 4\n\t"
    "csrw mepc, t0\n\t"
    "mret"
    );
}

void test_lr() {
    printf("----------test lr----------\n");

    // Test 1: trigger off, normal lr
    printf("test1:");
    asm volatile(
        "lr.d t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE);

    // Test 2: trigger on, lr from trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "lr.d t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP);

    // Test 3: trigger on, lr from non trigger address
    printf("test3:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD);
    asm volatile(
        "lr.d t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE);
}

void test_sc() {
    printf("----------test sc----------\n");

    // Test 1: trigger off, normal sc
    printf("test1:");
    asm volatile(
        "lr.d t0, (%0)\n\t"
        "sc.d t0, t1, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE);

    // Test 2: trigger on, sc to trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "lr.d t0, (%0)\n\t"
        "sc.d t0, t1, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP);

    // Test 3: trigger on, sc to non trigger address
    printf("test3:");
    setTrigger(0x82000000ULL, TRIGGER_STORE);
    asm volatile(
        "lr.d t0, (%0)\n\t"
        "sc.d t0, t1, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE);

    // Test 4: trigger on, failure sc to trigger address
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "sc.d t0, t1, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP);
}

void test_amo() {
    printf("----------test amo----------\n");

    // Test 1: trigger off, normal amo
    printf("test1:");
    asm volatile(
        "amoswap.d t1, t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE);

    // Test 2: trigger on, amo trigger from load address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "amoadd.d t1, t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP);

    // Test 3: trigger on, amo trigger from store address
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "amoand.d t1, t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP);

    // Test 4: trigger on, amo trigger from load and store address
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD_AND_STORE);
    asm volatile(
        "amomax.d t1, t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP);

    // Test 5: trigger on, amo trigger from non load and store address
    printf("test5:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD_AND_STORE);
    asm volatile(
        "amominu.d t1, t0, (%0)\n\t"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE);
}

int main() {
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

    test_lr();
    test_sc();
    test_amo();

    asm volatile("mv a0, %0; .word 0x0000006b" : :"r"(0));
    return 0;
}