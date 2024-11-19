/*
** It may be useless because it may be a wrong test invloved with the function call and asm inlined.
*/

#include <klib.h>
#include <stdint.h>
#define W_32  0b010
#define W_64  0b011
#define W_128 0b100
#define CAUSE_NONE 0
#define CAUSE_II   2
#define X0 0
#define T0 5
#define T1 6
#define T2 7
#define T3 28
#define T4 29
#define T5 30
#define T6 31

void check_and_print(int r_cause, uint64_t rd, uint64_t rd_exp, uint64_t md, uint64_t md_exp){
    uint64_t mcause;
    asm volatile( "csrr %0, mcause\n\t"
                  "csrwi mcause, 0\n\t"
                  :"=r"(mcause):);
    if(r_cause == mcause && rd == rd_exp && md == md_exp)
        printf("\033[40;32;5m PASS: \033[0m");
    else
        printf("\033[40;31;5m FAIL: \033[0m");
    printf("mcause: %d rd: %lx rd_exp: %lx md: %lx md_exp: %lx\n", mcause, rd, rd_exp, md, md_exp);
}

uint32_t gen_instr(int rd, int rs1, int rs2, int funct3) {
    return 0x2f | (rd << 7) | (funct3 << 12) | (rs1 << 15) | (rs2 << 20) | (0x14 << 25);
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

void test_amocas() {
    printf("----------test amocas----------\n");
    uint32_t instr;
    uint64_t rd, rd_1, md, md_1;
    uint64_t ra;

    asm volatile(
        "mv %0, ra\n\t"
        "li t0, 0x81000000\n\t"
        "sd ra, -8(t0)\n\t"
        : "=r"(ra)
    );
    printf("store ra: %lx\n", ra);

    // Test 1: fail cas word
    printf("test1:");
    instr = gen_instr(T2, T0, T1, W_32);
    asm volatile(
        "li t0, 0x81000000\n\t"
        "li t1, 5\n\t"
        "li t2, 10\n\t"
        "sw t2, 0(t0)\n\t"
        "li t2, 0\n\t"
        ".word %2\n\t"
        "mv %0, t2\n\t"
        "lw %1, 0(t0)\n\t"
        : "=r"(rd), "=r"(md)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 10, md, 10);

    // Test 2: success cas word
    printf("test2:");
    instr = gen_instr(T2, T0, T1, W_32);
    asm volatile(
        "li t0, 0x81000000\n\t"
        "li t1, 5\n\t"
        "li t2, 10\n\t"
        "sw t2, 0(t0)\n\t"
        ".word %2\n\t"
        "mv %0, t2\n\t"
        "lw %1, 0(t0)\n\t"
        : "=r"(rd), "=r"(md)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 10, md, 5);

    // Test 3: fail cas doubleword
    printf("test3:");
    instr = gen_instr(T2, T0, T1, W_64);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000005\n\t"
        "li t2, 0x80000000a\n\t"
        "sd t2, 0(t0)\n\t"
        "li t2, 0\n\t"
        ".word %2\n\t"
        "mv %0, t2\n\t"
        "ld %1, 0(t0)\n\t"
        : "=r"(rd), "=r"(md)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 0x80000000aULL, md, 0x80000000aULL);

    // Test 4: success cas doubleword
    printf("test4:");
    instr = gen_instr(T2, T0, T1, W_64);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000005\n\t"
        "li t2, 0x80000000a\n\t"
        "sd t2, 0(t0)\n\t"
        ".word %2\n\t"
        "mv %0, t2\n\t"
        "ld %1, 0(t0)\n\t"
        : "=r"(rd), "=r"(md)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 0x80000000aULL, md, 0x800000005ULL);

    // Test 5: success cas quadword
    printf("test5:");
    instr = gen_instr(T3, T0, T1, W_128);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000005\n\t"
        "li t2, 0x80000000a\n\t"
        "li t3, 0x800000004\n\t"
        "li t4, 0x800000009\n\t"
        "sd t3, 0(t0)\n\t"
        "sd t4, 8(t0)\n\t"
        "li t3, 0\n\t"
        "li t4, 0\n\t"
        ".word %4\n\t"
        "mv %0, t3\n\t"
        "mv %1, t4\n\t"
        "ld %2, 0(t0)\n\t"
        "ld %3, 8(t0)\n\t"
        : "=r"(rd), "=r"(rd_1), "=r"(md), "=r"(md_1)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 0x800000004ULL, md, 0x800000004ULL);
    printf("test5:");
    check_and_print(CAUSE_NONE, rd_1, 0x800000009ULL, md_1, 0x800000009ULL);

    // Test 6: fail cas quadword
    printf("test6:");
    instr = gen_instr(T3, T0, T1, W_128);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000005\n\t"
        "li t2, 0x80000000a\n\t"
        "li t3, 0x800000004\n\t"
        "li t4, 0x800000009\n\t"
        "sd t3, 0(t0)\n\t"
        "sd t4, 8(t0)\n\t"
        ".word %4\n\t"
        "mv %0, t3\n\t"
        "mv %1, t4\n\t"
        "ld %2, 0(t0)\n\t"
        "ld %3, 8(t0)\n\t"
        : "=r"(rd), "=r"(rd_1), "=r"(md), "=r"(md_1)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 0x800000004ULL, md, 0x800000005ULL);
    printf("test6:");
    check_and_print(CAUSE_NONE, rd_1, 0x800000009ULL, md_1, 0x80000000aULL);

    // Test 7: fail cas quadword, rd = x0
    printf("test7:");
    instr = gen_instr(X0, T0, T1, W_128);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000005\n\t"
        "li t2, 0x80000000a\n\t"
        "li ra, 0x800000009\n\t"
        "sd x0, 0(t0)\n\t"
        "sd ra, 8(t0)\n\t"
        ".word %4\n\t"
        "mv %0, x0\n\t"
        "mv %1, ra\n\t"
        "ld %2, 0(t0)\n\t"
        "ld %3, 8(t0)\n\t"
        : "=r"(rd), "=r"(rd_1), "=r"(md), "=r"(md_1)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 0ULL, md, 0ULL);
    printf("test7:");
    check_and_print(CAUSE_NONE, rd_1, 0x800000009ULL, md_1, 0x800000009ULL);

    // Test 8: success cas quadword, rd = x0
    printf("test8:");
    instr = gen_instr(X0, T0, T1, W_128);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000005\n\t"
        "li t2, 0x80000000a\n\t"
        "sd x0, 0(t0)\n\t"
        "sd x0, 8(t0)\n\t"
        "li ra, 0x800000009\n\t"
        ".word %4\n\t"
        "mv %0, x0\n\t"
        "mv %1, ra\n\t"
        "ld %2, 0(t0)\n\t"
        "ld %3, 8(t0)\n\t"
        : "=r"(rd), "=r"(rd_1), "=r"(md), "=r"(md_1)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 0ULL, md, 0x800000005ULL);
    printf("test8:");
    check_and_print(CAUSE_NONE, rd_1, 0x800000009ULL, md_1, 0x80000000aULL);

    // Test 9: success cas quadword, rs2 = x0
    printf("test9:");
    instr = gen_instr(T1, T0, X0, W_128);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000004\n\t"
        "li t2, 0x800000009\n\t"
        "li ra, 0x800000009\n\t"
        "sd t1, 0(t0)\n\t"
        "sd t2, 8(t0)\n\t"
        ".word %4\n\t"
        "mv %0, t1\n\t"
        "mv %1, t2\n\t"
        "ld %2, 0(t0)\n\t"
        "ld %3, 8(t0)\n\t"
        : "=r"(rd), "=r"(rd_1), "=r"(md), "=r"(md_1)
        : "i"(instr)
    );
    check_and_print(CAUSE_NONE, rd, 0x800000004ULL, md, 0ULL);
    printf("test9:");
    check_and_print(CAUSE_NONE, rd_1, 0x800000009ULL, md_1, 0ULL);

    // Test 10: fail cas quadword, rs2 raise EX_II
    printf("test10:");
    instr = gen_instr(T1, T0, T4, W_128);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t4, 0x800000005\n\t"
        "li t5, 0x80000000a\n\t"
        "li t1, 0x800000004\n\t"
        "li t2, 0x800000009\n\t"
        "sd t1, 0(t0)\n\t"
        "sd t2, 8(t0)\n\t"
        ".word %4\n\t"
        "mv %0, t1\n\t"
        "mv %1, t2\n\t"
        "li t0, 0x81000000\n\t"
        "ld %2, 0(t0)\n\t"
        "ld %3, 8(t0)\n\t"
        : "=r"(rd), "=r"(rd_1), "=r"(md), "=r"(md_1)
        : "i"(instr)
    );
    check_and_print(CAUSE_II, rd, 0x800000004ULL, md, 0x800000004ULL);
    printf("test10:");
    check_and_print(CAUSE_NONE, rd_1, 0x800000009ULL, md_1, 0x800000009ULL);

    // Test 11: fail cas quadword, rd raise EX_II
    printf("test11:");
    instr = gen_instr(T4, T0, T1, W_128);
    asm volatile(
        "li t0,  0x81000000\n\t"
        "li t1, 0x800000005\n\t"
        "li t2, 0x80000000a\n\t"
        "li t4, 0x800000004\n\t"
        "li t5, 0x800000009\n\t"
        "sd t4, 0(t0)\n\t"
        "sd t5, 8(t0)\n\t"
        ".word %4\n\t"
        "mv %0, t4\n\t"
        "mv %1, t5\n\t"
        "li t0, 0x81000000\n\t" // t0 is used to read/write mepc, so we should re li
        "ld %2, 0(t0)\n\t"
        "ld %3, 8(t0)\n\t"
        : "=r"(rd), "=r"(rd_1), "=r"(md), "=r"(md_1)
        : "i"(instr)
    );
    check_and_print(CAUSE_II, rd, 0x800000004ULL, md, 0x800000004ULL);
    printf("test11:");
    check_and_print(CAUSE_NONE, rd_1, 0x800000009ULL, md_1, 0x800000009ULL);

    asm volatile(
        "li t0, 0x81000000\n\t"
        "ld ra, -8(t0)\n\t"
        "mv %0, ra\n\t"
        : "=r"(ra)
    );
    printf("load ra: %lx\n", ra);
}

int main() {
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

    test_amocas();

    asm volatile("mv a0, %0; .word 0x0000006b" : :"r"(0));
    return 0;
}