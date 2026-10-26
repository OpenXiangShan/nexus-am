#include <klib.h>

#define TARGET_PC       0x0003FFFFFFFFFFC0UL // highest canonical Sv48x4 GPA
#define CALLEE_PC       (TARGET_PC - 0x200UL)
#define ENTRY_PC        (TARGET_PC - 0x100UL)
#define CALL_PC         (TARGET_PC - 0x4UL)
#define WARM_BLOCK_PC   (TARGET_PC - 0x1000UL)
#define WARM_NOP_SIZE   (0x30UL)
#define WARM_CALL_PC    (WARM_BLOCK_PC + WARM_NOP_SIZE)
#define WARM_DONE_PC    (WARM_CALL_PC + 0xCUL) // magic 0xC: 3 insts before jump
#define ILLEGAL_INSN    0x0U

#define ROOT_PA         0x82000000UL // Sv48x4 root, 16 KiB aligned
#define TABLE0_PA       0x82004000UL
#define TABLE1_PA       0x82005000UL
#define LEAF_PA         0x81000000UL // 2 MiB leaf, top of canonical GPA range

#define ROOT_ENTRIES    2048
#define TABLE_ENTRIES   512

#define PTE_V           (1UL << 0)
#define PTE_R           (1UL << 1)
#define PTE_W           (1UL << 2)
#define PTE_X           (1UL << 3)
#define PTE_U           (1UL << 4)
#define PTE_A           (1UL << 6)
#define PTE_D           (1UL << 7)
#define PTE_LEAF_FLAGS  (PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D)

#define MSTATUS_MPP_MASK  (3UL << 11)
#define MSTATUS_MPP_S     (1UL << 11)
#define MSTATUS_MPV       (1UL << 39)
#define HSTATUS_VSXL_MASK (3UL << 32)
#define HSTATUS_VSXL_64   (2UL << 32)
#define MISA_H            (1UL << ('H' - 'A'))

#define MCAUSE_ILL         2UL
#define HGATP_MODE_SV48X4  9UL

#define LEAF_GPA_BASE (TARGET_PC & ~((1UL << 21) - 1))
#define GPA_TO_PA(pc) (LEAF_PA + ((pc) - LEAF_GPA_BASE))

_Static_assert((TARGET_PC >> 49) & 1UL,
               "the target must exercise a set GPA bit 49");
_Static_assert((TARGET_PC >> 50) == 0,
               "the target must remain canonical in Sv48x4");
_Static_assert((ROOT_PA & 0x3FFFUL) == 0,
               "the Sv48x4 root page table must be 16 KiB aligned");
_Static_assert((LEAF_PA & 0x1FFFFFUL) == 0,
               "the level-1 leaf must represent a 2 MiB page");
_Static_assert(CALLEE_PC < ENTRY_PC && ENTRY_PC < CALL_PC,
               "the call path must not overlap the callee or the target");
_Static_assert((WARM_BLOCK_PC & 0x3FUL) == 0,
               "the warm-up sequence must start on a fetch-block boundary");

#define STR1(x) #x
#define STR(x) STR1(x)

#define read_csr(csr)                                           \
    ({                                                          \
        uint64_t value;                                         \
        asm volatile("csrr %0, " STR(csr) : "=r"(value)); \
        value;                                                  \
    })

#define write_csr(csr, value)                                          \
    do {                                                               \
        uint64_t csr_value = (uint64_t)(value);                        \
        asm volatile("csrw " STR(csr) ", %0" :: "r"(csr_value)); \
    } while (0)

volatile uint64_t trap_mcause;
volatile uint64_t trap_mepc;
volatile uint64_t trap_mtval;
volatile uint64_t trap_mtval2;
volatile uint32_t trap_count;
volatile uint64_t resume_pc;
extern char trap_entry[];
__asm__(
".globl trap_entry\n"
".align 2\n"
"trap_entry:\n"
"  csrr t0, mcause\n"
"  la   t1, trap_mcause\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, mepc\n"
"  la   t1, trap_mepc\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, mtval\n"
"  la   t1, trap_mtval\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, 0x34b\n"             // mtval2
"  la   t1, trap_mtval2\n"
"  sd   t0, 0(t1)\n"
"  la   t1, trap_count\n"
"  lw   t0, 0(t1)\n"
"  addi t0, t0, 1\n"
"  sw   t0, 0(t1)\n"
"  la   t1, resume_pc\n"
"  ld   t0, 0(t1)\n"
"  csrw mepc, t0\n"
"  li   t0, 0x1800\n"           // MPP=M, so MRET returns to M-mode
"  csrs mstatus, t0\n"
"  mret\n"
);

static uint64_t pte_leaf(uint64_t pa) {
    return (pa >> 2) | PTE_LEAF_FLAGS;
}

static uint64_t pte_table(uint64_t pa) {
    return (pa >> 2) | PTE_V;
}

static uint32_t encode_jal(uint32_t rd, int32_t offset) {
    uint32_t imm = (uint32_t)offset & 0x1FFFFFU;
    return ((imm >> 20) & 0x1U) << 31 |
           ((imm >> 1) & 0x3FFU) << 21 |
           ((imm >> 11) & 0x1U) << 20 |
           ((imm >> 12) & 0xFFU) << 12 |
           (rd & 0x1FU) << 7 |
           0x6FU;
}

static uint32_t encode_bne(uint32_t rs1, uint32_t rs2, int32_t offset) {
    uint32_t imm = (uint32_t)offset & 0x1FFFU;
    return ((imm >> 12) & 0x1U) << 31 |
           ((imm >> 5) & 0x3FU) << 25 |
           (rs2 & 0x1FU) << 20 |
           (rs1 & 0x1FU) << 15 |
           0x1U << 12 |
           ((imm >> 1) & 0xFU) << 8 |
           ((imm >> 11) & 0x1U) << 7 |
           0x63U;
}

static void clear_table(volatile uint64_t *table, uint32_t entries) {
    for (uint32_t i = 0; i < entries; i++) {
        table[i] = 0;
    }
}

static int enable_sv48x4(void) {
    if ((read_csr(misa) & MISA_H) == 0) {
        return 0;
    }

    uint64_t hstatus = read_csr(hstatus);
    hstatus = (hstatus & ~HSTATUS_VSXL_MASK) | HSTATUS_VSXL_64;
    write_csr(hstatus, hstatus);
    if ((read_csr(hstatus) & HSTATUS_VSXL_MASK) != HSTATUS_VSXL_64) {
        return 0;
    }

    write_csr(vsatp, 0);
    write_csr(hgatp, (HGATP_MODE_SV48X4 << 60) | (ROOT_PA >> 12));
    asm volatile(".word 0x62000073" ::: "memory"); // hfence.gvma x0, x0
    return (read_csr(hgatp) >> 60) == HGATP_MODE_SV48X4;
}

static void enter_vs(uint64_t pc) {
    uint64_t mstatus = read_csr(mstatus);
    mstatus = (mstatus & ~MSTATUS_MPP_MASK) | MSTATUS_MPP_S;
    mstatus |= MSTATUS_MPV;
    write_csr(mstatus, mstatus);

    asm volatile(
        "la   t0, 1f\n\t"
        "sd   t0, resume_pc, t1\n\t"
        "csrw mepc, %0\n\t"
        "mret\n\t"
        "1:\n\t"
        :: "r"(pc) : "t0", "t1", "memory");
}

int main(void) {
    // pre-check
    if (!enable_sv48x4()) {
        printf("UNSUPPORTED: H extension and Sv48x4 are required\n");
        return 0;
    }

    // Build the Sv48x4 page table at highest canonical GPA addresses
    volatile uint64_t *root = (volatile uint64_t *)ROOT_PA;
    volatile uint64_t *level0 = (volatile uint64_t *)TABLE0_PA;
    volatile uint64_t *level1 = (volatile uint64_t *)TABLE1_PA;

    clear_table(root, ROOT_ENTRIES);
    clear_table(level0, TABLE_ENTRIES);
    clear_table(level1, TABLE_ENTRIES);

    uint64_t root_index = (TARGET_PC >> 39) & (ROOT_ENTRIES - 1);
    uint64_t level0_index = (TARGET_PC >> 30) & (TABLE_ENTRIES - 1);
    uint64_t level1_index = (TARGET_PC >> 21) & (TABLE_ENTRIES - 1);

    root[root_index] = pte_table(TABLE0_PA);
    level0[level0_index] = pte_table(TABLE1_PA);
    level1[level1_index] = pte_leaf(LEAF_PA);
    asm volatile("fence rw, rw" ::: "memory");

    // setup PMP
    write_csr(pmpaddr0, -1L);
    write_csr(pmpcfg0, 0x1FUL); // NAPOT | RWX
    // setup trap
    write_csr(medeleg, 0);
    write_csr(mideleg, 0);
    write_csr(mtvec, (uint64_t)trap_entry);

    // BTB & RAS training sequence: jump to callee 4 times, then goto entry
    volatile uint32_t *p = (volatile uint32_t *) GPA_TO_PA(WARM_BLOCK_PC);
    *(p++) = 0x00400293U; // li t0, 4
    while (p < (volatile uint32_t *)GPA_TO_PA(WARM_CALL_PC)) {
        *(p++) = 0x00000013U; // nop
    }
    *(p++) = encode_jal(1, (int32_t)(CALLEE_PC - WARM_CALL_PC)); // jal ra, callee
    *(p++) = 0xfff28293U; // addi t0, t0, -1
    *(p++) = encode_bne(5, 0, -8); // bne t0, zero, warm_call_pc
    *p = encode_jal(0, (int32_t)(ENTRY_PC - WARM_DONE_PC)); // jal zero, entry
    assert(p == (volatile uint32_t *)GPA_TO_PA(WARM_DONE_PC));

    // entry sequence: jump to callee, which returns to the illegal target
    p = (volatile uint32_t *) GPA_TO_PA(ENTRY_PC);
    while (p < (volatile uint32_t *) GPA_TO_PA(CALL_PC)) {
        *(p++) = 0x00000013U; // nop
    }
    *(p++) = encode_jal(1, (int32_t)(CALLEE_PC - CALL_PC)); // jal ra, callee
    *p = ILLEGAL_INSN;
    assert(p == (volatile uint32_t *)GPA_TO_PA(TARGET_PC));

    // callee
    *(volatile uint32_t *)GPA_TO_PA(CALLEE_PC) = 0x00008067U; // ret

    asm volatile("fence rw, rw; fence.i" ::: "memory");

    trap_mcause = trap_mepc = trap_mtval = trap_mtval2 = 0;
    trap_count = 0;
    resume_pc = 0;

    // goto the warm-up sequence in vs mode
    enter_vs(WARM_BLOCK_PC);

    printf("Caught traps=%u, mcause=0x%lx, mepc=0x%lx, mtval=0x%lx, mtval2=0x%lx\n",
           trap_count, trap_mcause, trap_mepc, trap_mtval, trap_mtval2);

    int pass = trap_count == 1 &&
               trap_mcause == MCAUSE_ILL &&
               trap_mepc == TARGET_PC &&
               trap_mtval == ILLEGAL_INSN &&
               trap_mtval2 == 0;

    if (pass) {
        printf("PASS!\n");
        return 0;
    }

    // The historical bug sets bit 50 by sign-extending bit 49.
    if (trap_mepc == (TARGET_PC | (1UL << 50))) {
        printf("FAIL: mepc has a spurious bit 50\n");
    } else {
        printf("FAIL: exception state does not identify the faulting PC\n");
    }
    return 1;
}
