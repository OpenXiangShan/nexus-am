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
#define __riscv_xlen 64
#define TDATA1_TYPE 6ULL // mcontrol6
#define CAUSE_NONE 0
#define CAUSE_BP   3

#define INIT() \
do{ \
    asm volatile ( \
        "lui a0,0x2\n" \
        "addiw a0,a0,512\n" \
        "csrs mstatus,a0\n" \
        "csrwi vcsr,0"\
        :: \
    ); \
} while (0)

void check_and_print(int r_cause, int r_vl, int r_start){
    uint64_t mcause;
    uint64_t vstart;
    uint64_t vl;
    asm volatile("csrr %0, mcause\n\t"
                  "csrr %1, vl\n\t"
                  "csrr %2, vstart\n\t"
                  "csrwi mcause, 0\n\t"
                  "csrwi tdata1, 0\n\t"
                  :"=r"(mcause), "=r"(vl), "=r"(vstart):);
    if(r_cause == mcause && r_vl == vl && r_start == vstart)
        printf("\033[40;32;5m PASS: \033[0m");
    else
        printf("\033[40;31;5m FAIL: \033[0m");
    printf("mcause: %d, vl: %d, vstart: %d \n", mcause, vl, vstart);
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

void test_vle() {
    printf("----------test vle----------\n");

    // Test 1: trigger off, normal load
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, load from trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, load from trigger address - 1
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v0,(%0)"
        :
        :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, load from trigger address - 4
    printf("test4:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v0,(%0)"
        :
        :"r"(0x82000000ULL-4)
    );
    check_and_print(CAUSE_BP, 8, 4);

    // Test 5: trigger off, load from trigger address - 4
    printf("test5:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v1,(%0)"
        :
        :"r"(0x82000000ULL-4)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 6: trigger on, load from trigger address - 8
    printf("test6:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v0,(%0)"
        :
        :"r"(0x82000000ULL-8)
    );
    check_and_print(CAUSE_NONE, 8, 0);
}

void test_vlm() {
    printf("----------test vlm----------\n");

    // Test 1: trigger off, normal mask load
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlm.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, mask load from trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlm.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, mask load from trigger address
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,16,e8,m1\n\t"
        "vlm.v v0,(%0)"
        :
        :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 16, 1);

    // Test 4: trigger on, mask load from trigger address
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,12,e8,m1\n\t"
        "vlm.v v0,(%0)"
        :
        :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 12, 1);
}

void test_vlse() {
    printf("----------test vlse----------\n");

    // Test 1: trigger off, normal strided load
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlse8.v v0,(%0),%1"
        :
        :"r"(0x81000000ULL), "r"(2)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, strided load from trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlse8.v v0,(%0),%1"
        :
        :"r"(0x81000000ULL), "r"(2)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, strided load from trigger address - stride
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlse8.v v0,(%0),%1"
        :
        :"r"(0x81000000ULL-2), "r"(2)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, strided load from trigger address - 4 * stride
    printf("test4:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlse8.v v0,(%0),%1"
        :
        :"r"(0x82000000ULL-8), "r"(2)
    );
    check_and_print(CAUSE_BP, 8, 4);

    // Test 5: trigger off, strided load from address - 4 * stride
    printf("test5:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlse8.v v1,(%0),%1"
        :
        :"r"(0x82000000ULL-8), "r"(2)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 6: trigger on, strided load from address - 8 * stride
    printf("test6:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vlse8.v v0,(%0),%1"
        :
        :"r"(0x82000000ULL-16), "r"(2)
    );
    check_and_print(CAUSE_NONE, 8, 0);
}

void test_vlxe() {
    printf("----------test vlxe----------\n");

    char idx[8] = {0, 5, 4, 3, 1, 2, 7, 6};
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v1, (%0)\n\t"
        :
        :"r"(idx)
    );
    // Test 1: trigger off, normal indexed load
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vloxei8.v v0, (%0), v1"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, indexed load from trigger address
    printf("test1:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vloxei8.v v0, (%0), v1"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, indexed load from trigger address - 7
    printf("test3:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vloxei8.v v0, (%0), v1"
        :
        :"r"(0x82000000ULL-7)
    );
    check_and_print(CAUSE_BP, 8, 6);

    // Test 4: trigger off, indexed load from address - 7
    printf("test4:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vloxei8.v v0, (%0), v1"
        :
        :"r"(0x82000000ULL-7)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 5: trigger on, indexed load from address - 8
    printf("test5:");
    setTrigger(0x82000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vloxei8.v v0, (%0), v1"
        :
        :"r"(0x82000000ULL-8)
    );
    check_and_print(CAUSE_NONE, 8, 0);
}

void test_vleff() {
    printf("----------test vleff----------\n");

    // Test 1: trigger off, normal fof load
    printf("test1:");
    asm volatile (
      "vsetivli zero,16,e8,m1\n\t"
      "vle8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
      :
    );
    check_and_print(CAUSE_NONE, 16, 0);

    // Test 2: trigger on, fof load from trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,16,e8,m1\n\t"
      "vle8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
      :
    );
    check_and_print(CAUSE_BP, 16, 0);

    // Test 3: trigger on, for load from trigger address - 3
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,16,e8,m1\n\t"
      "vle8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL-3)
      :
    );
    check_and_print(CAUSE_NONE, 3, 0);
}

void test_vlseg() {
    printf("----------test vlseg----------\n");

    // Test 1: trigger off, normal unit seg load
    printf("test1:");
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, unit seg load
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, unit seg load
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL-2)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, unit seg load
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 8, 0);

}

void test_vlsegff() {
    printf("----------test vlsegff----------\n");

    // Test 1: trigger off, normal fof unit seg load
    printf("test1:");
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, fof unit seg load
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, fof unit seg load
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL-4)
    );
    check_and_print(CAUSE_NONE, 2, 0);

    // Test 4: trigger on, fof unit seg load
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 5: trigger on, fof unit seg load
    printf("test5:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlseg2e8ff.v v0,(%0)"
      :
      :"r"(0x81000000ULL-3)
    );
    check_and_print(CAUSE_NONE, 1, 0);
}

void test_vlsseg() {
    printf("----------test vlsseg----------\n");

    // Test 1: trigger off, normal strided seg load
    printf("test1:");
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlsseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL), "r"(10)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, strided seg load
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlsseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL), "r"(10)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, strided seg load
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlsseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL-10), "r"(10)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, strided seg load
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vlsseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL-11), "r"(10)
    );
    check_and_print(CAUSE_BP, 8, 1);
}

void test_vlxseg() {
    printf("----------test vlxseg----------\n");

    char idx[8] = {0, 50, 40, 30, 10, 20, 70, 60};
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v1, (%0)\n\t"
        :
        :"r"(idx)
    );

    // Test 1: trigger off, normal indexed seg load
    printf("test1:");
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vloxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, indexed seg load
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vloxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, indexed seg load
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vloxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL-40)
    );
    check_and_print(CAUSE_BP, 8, 2);

    // Test 4: trigger on, indexed seg load
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vloxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL-71)
    );
    check_and_print(CAUSE_BP, 8, 6);
}

void test_vlr() {
    printf("----------test vlr----------\n");

    // Test 1: trigger off, normal whole load
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vl1re8.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, whole load
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vl1re8.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, whole load
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_LOAD);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vl4re8.v v0,(%0)"
        :
        :"r"(0x81000000ULL-16)
    );
    check_and_print(CAUSE_BP, 8, 16);
}

void test_vse() {
    printf("----------test vse----------\n");

    // Test 1: trigger off, normal store
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vse8.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, store to trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vse8.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, store to trigger address - 1
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vse8.v v0,(%0)"
        :
        :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, store to trigger address - 4
    printf("test4:");
    setTrigger(0x82000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vse8.v v0,(%0)"
        :
        :"r"(0x82000000ULL-4)
    );
    check_and_print(CAUSE_BP, 8, 4);

    // Test 5: trigger off, store to trigger address - 4
    printf("test5:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vse8.v v1,(%0)"
        :
        :"r"(0x82000000ULL-4)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 6: trigger on, store to trigger address - 8
    printf("test6:");
    setTrigger(0x82000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vse8.v v0,(%0)"
        :
        :"r"(0x82000000ULL-8)
    );
    check_and_print(CAUSE_NONE, 8, 0);
}

void test_vsm() { 
    printf("----------test vsm----------\n");

    // Test 1: trigger off, normal mask store
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsm.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, mask store to trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsm.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, mask store to trigger address - 1
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,16,e8,m1\n\t"
        "vsm.v v0,(%0)"
        :
        :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 16, 1);

    // Test 4: trigger on, mask store to trigger address - 1
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,14,e8,m1\n\t"
        "vsm.v v0,(%0)"
        :
        :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 14, 1);
}

void test_vsse() {
    printf("----------test vsse----------\n");

    // Test 1: trigger off, normal strided store
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsse8.v v0,(%0),%1"
        :
        :"r"(0x81000000ULL), "r"(2)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, strided store to trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsse8.v v0,(%0),%1"
        :
        :"r"(0x81000000ULL), "r"(2)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, strided store to trigger address - stride
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsse8.v v0,(%0),%1"
        :
        :"r"(0x81000000ULL-2), "r"(2)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, strided store to trigger address - 4 * stride
    printf("test4:");
    setTrigger(0x82000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsse8.v v0,(%0),%1"
        :
        :"r"(0x82000000ULL-8), "r"(2)
    );
    check_and_print(CAUSE_BP, 8, 4);

    // Test 5: trigger off, strided store to address - 4 * stride
    printf("test5:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsse8.v v1,(%0),%1"
        :
        :"r"(0x82000000ULL-8), "r"(2)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 6: trigger on, strided store to address - 8 * stride
    printf("test6:");
    setTrigger(0x82000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsse8.v v0,(%0),%1"
        :
        :"r"(0x82000000ULL-16), "r"(2)
    );
    check_and_print(CAUSE_NONE, 8, 0);
}

void test_vsxe() {
    printf("----------test vsxe----------\n");

    char idx[8] = {0, 5, 4, 3, 1, 2, 7, 6};
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v1, (%0)\n\t"
        :
        :"r"(idx)
    );

    // Test 1: trigger off, normal indexed store
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsoxei8.v v0, (%0), v1"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, indexed store to trigger address
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsoxei8.v v0, (%0), v1"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, indexed store to address - 7
    printf("test3:");
    setTrigger(0x82000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsoxei8.v v0, (%0), v1"
        :
        :"r"(0x82000000ULL-7)
    );
    check_and_print(CAUSE_BP, 8, 6);

    // Test 4: trigger off, indexed store to address - 7
    printf("test4:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsoxei8.v v0, (%0), v1"
        :
        :"r"(0x82000000ULL-7)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 5: trigger on, indexed store to address - 8
    printf("test5:");
    setTrigger(0x82000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vsoxei8.v v0, (%0), v1"
        :
        :"r"(0x82000000ULL-8)
    );
    check_and_print(CAUSE_NONE, 8, 0);
}

void test_vsseg() {
    printf("----------test vsseg----------\n");

    // Test 1: trigger off, normal unit seg store
    printf("test1:");
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, unit seg store
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, unit seg store to address - stride
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL-2)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, unit seg store to address - 1
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsseg2e8.v v0,(%0)"
      :
      :"r"(0x81000000ULL-1)
    );
    check_and_print(CAUSE_BP, 8, 0);
}

void test_vssseg() {
    printf("----------test vssseg----------\n");

    // Test 1: trigger off, normal strided seg store
    printf("test1:");
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vssseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL), "r"(10)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, strided seg store
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vssseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL), "r"(10)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, strided seg store from address - 10
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vssseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL-10), "r"(10)
    );
    check_and_print(CAUSE_BP, 8, 1);

    // Test 4: trigger on, strided seg store from address - 11
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vssseg2e8.v v0,(%0),%1"
      :
      :"r"(0x81000000ULL-11), "r"(10)
    );
    check_and_print(CAUSE_BP, 8, 1);
}

void test_vsxseg() {
    printf("----------test vsxseg----------\n");

    char idx[8] = {0, 50, 40, 30, 10, 20, 70, 60};
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vle8.v v1, (%0)\n\t"
        :
        :"r"(idx)
    );

    // Test 1: trigger off, normal indexed seg store
    printf("test1:");
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsoxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, indexed seg store
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsoxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, indexed seg store
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsoxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL-40)
    );
    check_and_print(CAUSE_BP, 8, 2);

    // Test 4: trigger on, indexed seg store
    printf("test4:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile (
      "vsetivli zero,8,e8,m1\n\t"
      "vsoxseg2ei8.v v4,(%0),v1"
      :
      :"r"(0x81000000ULL-71)
    );
    check_and_print(CAUSE_BP, 8, 6);
}

void test_vsr() {
    printf("----------test vsr----------\n");

    // Test 1: trigger off, normal whole store
    printf("test1:");
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vs1r.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_NONE, 8, 0);

    // Test 2: trigger on, whole store
    printf("test2:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vs1r.v v0,(%0)"
        :
        :"r"(0x81000000ULL)
    );
    check_and_print(CAUSE_BP, 8, 0);

    // Test 3: trigger on, whole store with larger size
    printf("test3:");
    setTrigger(0x81000000ULL, TRIGGER_STORE);
    asm volatile(
        "vsetivli zero,8,e8,m1\n\t"
        "vs4r.v v0,(%0)"
        :
        :"r"(0x81000000ULL-16)
    );
    check_and_print(CAUSE_BP, 8, 16);
}

int main() {
    // init CTE
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
    INIT();

    test_vle();
    test_vlm();
    test_vlse();
    test_vlxe();
    test_vleff();
    test_vlseg();
    test_vlsegff();
    test_vlsseg();
    test_vlxseg();
    test_vlr();

    test_vse();
    test_vsm();
    test_vsse();
    test_vsxe();
    test_vsseg();
    test_vssseg();
    test_vsxseg();
    test_vsr();

    asm volatile("mv a0, %0; .word 0x0000006b" : :"r"(0));
    return 0;
}