// Test instruction fetch in the non-canonical address in Sv* modes.

// Usage: `make ARCH=riscv64-xs MODE=sv{39,48,57,39x4,48x4} JUMP={0,1,2,3} HALF_RVI={0,1} LOG_LEVEL={0,1,2}`
// MODE: Translation mode as RISC-V spec described.
// JUMP: How to enter the non-canonical space: 0=fall-through, 1=beqz, 2=jal, 3=jalr.
// HALF_RVI: Used with JUMP=0 to test the situation where a RVI (4B) instruction crosses the page boundary.

// Acknowledgement: Inspired by https://github.com/OpenXiangShan/XiangShan/issues/6264

#include <klib.h>

#ifndef MODE
#define MODE sv39
#endif
#ifndef HALF_RVI
#define HALF_RVI 0
#endif
#ifndef JUMP
#define JUMP 0
#endif

#if (HALF_RVI != 0) && (HALF_RVI != 1)
#error "HALF_RVI must be either 0 or 1"
#endif
#if (JUMP < 0) || (JUMP > 3)
#error "JUMP must be 0 (fall-through), 1 (beqz), 2 (jal), or 3 (jalr)"
#endif
#if HALF_RVI && JUMP
#error "HALF_RVI=1 is valid only when JUMP=0"
#endif

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define MODE_ID_sv39       1
#define MODE_ID_sv48       2
#define MODE_ID_sv57       3
#define MODE_ID_sv39x4     4
#define MODE_ID_sv48x4     5
#define MODE_ID_IMPL(mode) MODE_ID_##mode
#define MODE_ID(mode)      MODE_ID_IMPL(mode)
#define SELECTED_MODE      MODE_ID(MODE)

#if SELECTED_MODE == MODE_ID_sv39
#define MODE_NAME          "Sv39"
#define TEST_START         0x0000003FFFFFFFC0
#define FIRST_BAD_ADDR     0x0000004000000000
#define TRANSLATION_MODE   8
#define BOUNDARY_SHIFT     38
#define ROOT_PTE_COUNT     512
#define EXPECTED_CAUSE     12
#define VIRTUALIZED        0
#elif SELECTED_MODE == MODE_ID_sv48
#define MODE_NAME          "Sv48"
#define TEST_START         0x00007FFFFFFFFFC0
#define FIRST_BAD_ADDR     0x0000800000000000
#define TRANSLATION_MODE   9
#define BOUNDARY_SHIFT     47
#define ROOT_PTE_COUNT     512
#define EXPECTED_CAUSE     12
#define VIRTUALIZED        0
#elif SELECTED_MODE == MODE_ID_sv57
#define MODE_NAME          "Sv57"
#define TEST_START         0x00FFFFFFFFFFFFC0
#define FIRST_BAD_ADDR     0x0100000000000000
#define TRANSLATION_MODE   10
#define BOUNDARY_SHIFT     56
#define ROOT_PTE_COUNT     512
#define EXPECTED_CAUSE     12
#define VIRTUALIZED        0
#elif SELECTED_MODE == MODE_ID_sv39x4
#define MODE_NAME          "Sv39x4"
#define TEST_START         0x000001FFFFFFFFC0
#define FIRST_BAD_ADDR     0x0000020000000000
#define TRANSLATION_MODE   8
#define BOUNDARY_SHIFT     41
#define ROOT_PTE_COUNT     2048
#define EXPECTED_CAUSE     20
#define VIRTUALIZED        1
#elif SELECTED_MODE == MODE_ID_sv48x4
#define MODE_NAME          "Sv48x4"
#define TEST_START         0x0003FFFFFFFFFFC0
#define FIRST_BAD_ADDR     0x0004000000000000
#define TRANSLATION_MODE   9
#define BOUNDARY_SHIFT     50
#define ROOT_PTE_COUNT     2048
#define EXPECTED_CAUSE     20
#define VIRTUALIZED        1
#else
#error "Unknown MODE; expected sv39, sv48, sv57, sv39x4, or sv48x4"
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL 1
#endif

#if LOG_LEVEL <= 0
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#if LOG_LEVEL <= 1
#define INFO(...) printf(__VA_ARGS__)
#else
#define INFO(...)
#endif

#if LOG_LEVEL <= 2
#define CRITICAL(...) printf(__VA_ARGS__)
#else
#define CRITICAL(...)
#endif

#define ROOT_PA            0x82000000UL
#define TABLE0_PA          0x82004000UL
#define TABLE1_PA          0x82005000UL
#define TABLE2_PA          0x82006000UL
#define TABLE3_PA          0x82007000UL
#define TABLE4_PA          0x82008000UL
#define TABLE5_PA          0x82009000UL
#define FLAG_PA            0x82010000UL

#define BOUNDARY_LEAF_PA   0x81000000UL
#define BOUNDARY_CODE_PA   0x811FFFC0UL
#define ALIAS_CODE_PA      0x81200000UL
#define SLED_BYTES         0x40UL
#define TEST_ENTRY_ADDR    (TEST_START + 2UL * HALF_RVI)
#define JUMP_TARGET_ADDR   (FIRST_BAD_ADDR + 0x66UL)

#if JUMP == 0
#define FAULT_ADDR         FIRST_BAD_ADDR
#define EXPECTED_MEPC      (FIRST_BAD_ADDR - 2UL * HALF_RVI)
#define TRANSFER_NAME      "fall-through"
#elif JUMP == 1
#define FAULT_ADDR         JUMP_TARGET_ADDR
#define EXPECTED_MEPC      JUMP_TARGET_ADDR
#define TRANSFER_NAME      "beqz"
#elif JUMP == 2
#define FAULT_ADDR         JUMP_TARGET_ADDR
#define EXPECTED_MEPC      JUMP_TARGET_ADDR
#define TRANSFER_NAME      "jal"
#else
#define FAULT_ADDR         JUMP_TARGET_ADDR
#define EXPECTED_MEPC      JUMP_TARGET_ADDR
#define TRANSFER_NAME      "jalr"
#endif

#define PTE_V              (1UL << 0)
#define PTE_R              (1UL << 1)
#define PTE_W              (1UL << 2)
#define PTE_X              (1UL << 3)
#define PTE_U              (1UL << 4)
#define PTE_A              (1UL << 6)
#define PTE_D              (1UL << 7)
#define PTE_RWXAD          (PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D)
#define PTE_G_RWXAD        (PTE_RWXAD | PTE_U)

#define CSR_MSTATUS        0x300
#define CSR_MISA           0x301
#define CSR_MEDELEG        0x302
#define CSR_MIDELEG        0x303
#define CSR_MTVEC          0x305
#define CSR_SATP           0x180
#define CSR_VSATP          0x280
#define CSR_HSTATUS        0x600
#define CSR_HGATP          0x680

#define MSTATUS_MPP_MASK   (3UL << 11)
#define MSTATUS_MPP_S      (1UL << 11)
#define MSTATUS_MPV        (1UL << 39)
#define HSTATUS_VSXL_MASK  (3UL << 32)
#define HSTATUS_VSXL_64    (2UL << 32)
#define MISA_H             (1UL << ('H' - 'A'))

_Static_assert(TEST_START + SLED_BYTES == FIRST_BAD_ADDR,
               "The NOP sled must end at the first invalid address");
_Static_assert(BOUNDARY_CODE_PA + SLED_BYTES == ALIAS_CODE_PA,
               "The physical NOP sled and alias code must be adjacent");
_Static_assert((ROOT_PA & (VIRTUALIZED ? 0x3FFFUL : 0xFFFUL)) == 0,
               "The root page table is not sufficiently aligned");
_Static_assert((TEST_ENTRY_ADDR & 3UL) == (HALF_RVI ? 2UL : 0UL),
               "The test entry has the wrong instruction alignment");
_Static_assert(((JUMP_TARGET_ADDR - TEST_ENTRY_ADDR) & 1UL) == 0,
               "The direct-jump offset must be 2-byte aligned");
_Static_assert(JUMP_TARGET_ADDR - TEST_ENTRY_ADDR < 0x1000,
               "The beqz target is outside its immediate range");

#define read_csr(csr)                                           \
    ({                                                          \
        uint64_t value;                                         \
        asm volatile("csrr %0, " STRINGIFY(csr) : "=r"(value)); \
        value;                                                  \
    })

#define write_csr(csr, value)                                          \
    do {                                                               \
        uint64_t csr_value = (uint64_t)(value);                        \
        asm volatile("csrw " STRINGIFY(csr) ", %0" :: "r"(csr_value)); \
    } while (0)

volatile uint32_t trap_count;
volatile uint64_t last_mcause, last_mtval, last_mtval2, last_mepc;
volatile uint64_t m_continue;
volatile uint64_t ecall_mcause;
volatile uint64_t trap_resume;

__asm__(
".globl trap_handler\n"
".align 2\n"
"trap_handler:\n"
"  csrr t0, mcause\n"
"  li   t1, 9\n"
"  beq  t0, t1, .Lback_to_m\n"
"  li   t1, 10\n"
"  beq  t0, t1, .Lback_to_m\n"
"  li   t1, 11\n"
"  beq  t0, t1, .Lback_to_m\n"
"  la   t1, last_mcause\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, mtval\n"
"  la   t1, last_mtval\n"
"  sd   t0, 0(t1)\n"
#if VIRTUALIZED
"  csrr t0, 0x34b\n"       // mtval2
"  la   t1, last_mtval2\n"
"  sd   t0, 0(t1)\n"
#else
"  la   t1, last_mtval2\n"
"  sd   zero, 0(t1)\n"
#endif
"  csrr t0, mepc\n"
"  la   t1, last_mepc\n"
"  sd   t0, 0(t1)\n"
"  la   t1, trap_count\n"
"  lw   t0, 0(t1)\n"
"  addi t0, t0, 1\n"
"  sw   t0, 0(t1)\n"
"  la   t1, trap_resume\n"
"  ld   t0, 0(t1)\n"
"  bnez t0, .Lresume\n"
"  csrr t0, mepc\n"
"  addi t0, t0, 4\n"
"  csrw mepc, t0\n"
"  mret\n"
".Lresume:\n"
"  csrw mepc, t0\n"
"  mret\n"
".Lback_to_m:\n"
"  la   t1, ecall_mcause\n"
"  sd   t0, 0(t1)\n"
"  la   t1, m_continue\n"
"  ld   t0, 0(t1)\n"
"  csrw mepc, t0\n"
"  li   t0, 0x1800\n"
"  csrs mstatus, t0\n"
"  mret\n"
);
extern char trap_handler[];

__asm__(
".globl test_lower_entry\n"
".align 2\n"
"test_lower_entry:\n"
"  li   t0, " STRINGIFY(TEST_START) "\n"
#if HALF_RVI
"  addi t0, t0, 2\n"
#endif
"  jr   t0\n"
".globl test_lower_continue\n"
"test_lower_continue:\n"
"  ecall\n"
);
extern char test_lower_entry[];
extern char test_lower_continue[];

static uint64_t pte_leaf(uint64_t pa) {
    uint64_t flags = VIRTUALIZED ? PTE_G_RWXAD : PTE_RWXAD;
    return (pa >> 2) | flags;
}

static uint64_t pte_table(uint64_t pa) {
    return (pa >> 2) | PTE_V;
}

static void clear_table(volatile uint64_t *table, uint32_t entries) {
    for (uint32_t i = 0; i < entries; i++) table[i] = 0;
}

static void build_page_tables(void) {
    volatile uint64_t *root = (volatile uint64_t *)ROOT_PA;
    volatile uint64_t *t0 = (volatile uint64_t *)TABLE0_PA;
    volatile uint64_t *t1 = (volatile uint64_t *)TABLE1_PA;
    volatile uint64_t *t2 = (volatile uint64_t *)TABLE2_PA;
    volatile uint64_t *t3 = (volatile uint64_t *)TABLE3_PA;
    volatile uint64_t *t4 = (volatile uint64_t *)TABLE4_PA;
    volatile uint64_t *t5 = (volatile uint64_t *)TABLE5_PA;

    clear_table(root, ROOT_PTE_COUNT);
    clear_table(t0, 512);
    clear_table(t1, 512);
    clear_table(t2, 512);
    clear_table(t3, 512);
    clear_table(t4, 512);
    clear_table(t5, 512);

#if SELECTED_MODE == MODE_ID_sv39
    root[2] = pte_leaf(0x80000000UL);       // Low 1 GiB identity mapping.
    root[255] = pte_table(TABLE0_PA);
    t0[511] = pte_leaf(BOUNDARY_LEAF_PA);
    root[256] = pte_table(TABLE1_PA);       // Alias for bit 38 = 1.
    t1[0] = pte_leaf(ALIAS_CODE_PA);
#elif SELECTED_MODE == MODE_ID_sv48
    root[0] = pte_leaf(0);                  // Low 512 GiB identity mapping.
    root[255] = pte_table(TABLE0_PA);
    t0[511] = pte_table(TABLE1_PA);
    t1[511] = pte_leaf(BOUNDARY_LEAF_PA);
    root[256] = pte_table(TABLE2_PA);       // Alias for bit 47 = 1.
    t2[0] = pte_table(TABLE3_PA);
    t3[0] = pte_leaf(ALIAS_CODE_PA);
#elif SELECTED_MODE == MODE_ID_sv57
    root[0] = pte_leaf(0);                  // Low 256 TiB identity mapping.
    root[255] = pte_table(TABLE0_PA);
    t0[511] = pte_table(TABLE1_PA);
    t1[511] = pte_table(TABLE2_PA);
    t2[511] = pte_leaf(BOUNDARY_LEAF_PA);
    root[256] = pte_table(TABLE3_PA);       // Alias for bit 56 = 1.
    t3[0] = pte_table(TABLE4_PA);
    t4[0] = pte_table(TABLE5_PA);
    t5[0] = pte_leaf(ALIAS_CODE_PA);
#elif SELECTED_MODE == MODE_ID_sv39x4
    root[2] = pte_leaf(0x80000000UL);       // GPA 0x80000000 identity mapping.
    root[2047] = pte_table(TABLE0_PA);
    t0[511] = pte_leaf(BOUNDARY_LEAF_PA);
    root[0] = pte_table(TABLE1_PA);         // Alias if GPA bit 41 is dropped.
    t1[0] = pte_leaf(ALIAS_CODE_PA);
#elif SELECTED_MODE == MODE_ID_sv48x4
    root[2047] = pte_table(TABLE0_PA);
    t0[511] = pte_table(TABLE1_PA);
    t1[511] = pte_leaf(BOUNDARY_LEAF_PA);
    root[0] = pte_table(TABLE2_PA);         // Alias if GPA bit 50 is dropped.
    t2[0] = pte_table(TABLE3_PA);
    t3[0] = pte_leaf(ALIAS_CODE_PA);
    t2[2] = pte_leaf(0x80000000UL);         // GPA 0x80000000 identity mapping.
#endif

    asm volatile("fence rw, rw" ::: "memory");
}

static int enable_translation(void) {
#if VIRTUALIZED
    uint64_t hstatus = read_csr(CSR_HSTATUS);
    hstatus = (hstatus & ~HSTATUS_VSXL_MASK) | HSTATUS_VSXL_64;
    write_csr(CSR_HSTATUS, hstatus);
    if ((read_csr(CSR_HSTATUS) & HSTATUS_VSXL_MASK) != HSTATUS_VSXL_64)
        return 0;

    write_csr(CSR_VSATP, 0);                // VS-stage is Bare.
    write_csr(CSR_HGATP,
              ((uint64_t)TRANSLATION_MODE << 60) | (ROOT_PA >> 12));
    // hfence.gvma x0, x0; emitted directly because the project MARCH omits H.
    asm volatile(".word 0x62000073" ::: "memory");
    return (read_csr(CSR_HGATP) >> 60) == TRANSLATION_MODE;
#else
    write_csr(CSR_SATP,
              ((uint64_t)TRANSLATION_MODE << 60) | (ROOT_PA >> 12));
    asm volatile("sfence.vma" ::: "memory");
    return (read_csr(CSR_SATP) >> 60) == TRANSLATION_MODE;
#endif
}

static void enter_lower_mode(uint64_t entry) {
    uint64_t mstatus = read_csr(CSR_MSTATUS);
    mstatus = (mstatus & ~MSTATUS_MPP_MASK) | MSTATUS_MPP_S;
#if VIRTUALIZED
    mstatus |= MSTATUS_MPV;                  // MRET enters VS-mode.
#else
    mstatus &= ~MSTATUS_MPV;                 // MRET enters S/HS-mode.
#endif
    write_csr(CSR_MSTATUS, mstatus);
    asm volatile(
        "la t0, 1f\n\t"
        "sd t0, m_continue, t1\n\t"
        "csrw mepc, %0\n\t"
        "mret\n\t"
        "1:\n\t"
        :: "r"(entry) : "t0", "t1", "memory");
}

static void write_insn(uint64_t pa, uint32_t insn) {
    *(volatile uint16_t *)pa = (uint16_t)insn;
    *(volatile uint16_t *)(pa + 2) = (uint16_t)(insn >> 16);
}

#if JUMP == 1
static uint32_t encode_beqz(int32_t offset) {
    uint32_t imm = (uint32_t)offset;
    return ((imm & 0x1000U) << 19) |        // imm[12]
           ((imm & 0x07E0U) << 20) |        // imm[10:5]
           ((imm & 0x001EU) << 7)  |        // imm[4:1]
           ((imm & 0x0800U) >> 4)  |        // imm[11]
           0x00000063U;                     // beq x0,x0,offset
}
#elif JUMP == 2
static uint32_t encode_jal(int32_t offset) {
    uint32_t imm = (uint32_t)offset;
    return ((imm & 0x100000U) << 11) |      // imm[20]
           ((imm & 0x0007FEU) << 20) |      // imm[10:1]
           ((imm & 0x000800U) << 9)  |      // imm[11]
           (imm & 0x0FF000U)        |       // imm[19:12]
           (1U << 7) | 0x6FU;               // jal ra,offset
}
#elif JUMP == 3
static uint32_t encode_addi(uint32_t rd, uint32_t rs1, int32_t imm) {
    return (((uint32_t)imm & 0xFFFU) << 20) |
           (rs1 << 15) | (rd << 7) | 0x13U;
}

static uint32_t encode_slli(uint32_t rd, uint32_t rs1, uint32_t shamt) {
    return ((shamt & 0x3FU) << 20) |
           (rs1 << 15) | (1U << 12) | (rd << 7) | 0x13U;
}

static uint32_t encode_jalr(uint32_t rd, uint32_t rs1, int32_t imm) {
    return (((uint32_t)imm & 0xFFFU) << 20) |
           (rs1 << 15) | (rd << 7) | 0x67U;
}
#endif

static void install_test_code(void) {
    uint64_t off = HALF_RVI ? 2 : 0;
#if JUMP != 0
    uint64_t entry_pa = BOUNDARY_CODE_PA + off;
#endif
    for (uint64_t pa = BOUNDARY_CODE_PA + off; pa < BOUNDARY_CODE_PA + SLED_BYTES; pa += 4)
        write_insn(pa, 0x00000013U);             // nop

#if JUMP == 1
    write_insn(entry_pa, encode_beqz((int32_t)(FAULT_ADDR - TEST_ENTRY_ADDR)));
#elif JUMP == 2
    write_insn(entry_pa, encode_jal((int32_t)(FAULT_ADDR - TEST_ENTRY_ADDR)));
#elif JUMP == 3
    write_insn(entry_pa + 0x00, encode_addi(5, 0, 1));              // li t0,1
    write_insn(entry_pa + 0x04, encode_slli(5, 5, BOUNDARY_SHIFT)); // slli t0,t0,BOUNDARY_SHIFT
    write_insn(entry_pa + 0x08, encode_addi(5, 5, 0x66));           // addi t0,t0,0x66
    write_insn(entry_pa + 0x0C, encode_jalr(1, 5, 0));              // jalr ra,0(t0)
#endif

    // Set FLAG_PA to 0xFA, then ecall.  The shifts zero-extend the LUI result.
    uint64_t alias_entry = ALIAS_CODE_PA + (JUMP == 0 ? off : 0x66UL);
    write_insn(alias_entry + 0x00, 0x820102B7U); // lui t0,0x82010
    write_insn(alias_entry + 0x04, 0x02029293U); // slli t0,t0,32
    write_insn(alias_entry + 0x08, 0x0202D293U); // srli t0,t0,32
    write_insn(alias_entry + 0x0C, 0x0FA00313U); // li t1,0xfa
    write_insn(alias_entry + 0x10, 0x0062A023U); // sw t1,0(t0)
    write_insn(alias_entry + 0x14, 0x00000073U); // ecall
    *(volatile uint32_t *)FLAG_PA = 0;
    asm volatile("fence rw, rw; fence.i" ::: "memory");
}

int main(void) {
    CRITICAL("Test %s to the %s non-canonical address\n", TRANSFER_NAME, MODE_NAME);

#if VIRTUALIZED
    if ((read_csr(CSR_MISA) & MISA_H) == 0) {
        CRITICAL("UNSUPPORTED: hardware does not support H extension required by %s\n", MODE_NAME);
        return 0;
    }
#endif

    DEBUG("Building page tables\n");
    build_page_tables();

    // Full-range NAPOT PMP with RWX; reset PMP state may deny S/VS accesses.
    write_csr(0x3B0, -1L);                  // pmpaddr0
    write_csr(0x3A0, 0x1FUL);               // pmpcfg0 = NAPOT RWX
    write_csr(CSR_MEDELEG, 0);
    write_csr(CSR_MIDELEG, 0);
    write_csr(CSR_MTVEC, (uint64_t)trap_handler);

    DEBUG("Installing test code\n");
    install_test_code();

    DEBUG("Enabling %s translation mode\n", MODE_NAME);
    if (!enable_translation()) {
        CRITICAL("UNSUPPORTED: hardware does not support %s translation mode\n", MODE_NAME);
        return 0;
    }

    INFO("Jump to 0x%lx in %s-mode\n", (uint64_t)TEST_ENTRY_ADDR, VIRTUALIZED ? "VS" : "S");
    trap_count = 0;
    last_mcause = last_mtval = last_mtval2 = last_mepc = 0;
    ecall_mcause = 0;
    trap_resume = (uint64_t)test_lower_continue;
    enter_lower_mode((uint64_t)test_lower_entry);

    uint32_t flag = *(volatile uint32_t *)FLAG_PA;
    INFO("Caught ecall=%lu, traps=%u, mcause=%lu, mepc=0x%lx, mtval=0x%lx, mtval2=0x%lx, alias=0x%x\n",
           ecall_mcause, trap_count, last_mcause, last_mepc, last_mtval, last_mtval2, flag);

    int mtval_ok = last_mtval == 0 || last_mtval == FAULT_ADDR;
    int mtval2_ok = !VIRTUALIZED || last_mtval2 == 0 || last_mtval2 == (FAULT_ADDR >> 2);
    if (trap_count == 1 && last_mcause == EXPECTED_CAUSE && last_mepc == EXPECTED_MEPC && mtval_ok && mtval2_ok && flag == 0 && ecall_mcause == (VIRTUALIZED ? 10 : 9)) {
        CRITICAL("PASS!\n");
        return 0;
    }

    if (flag == 0xFA) {
        CRITICAL("FAIL: fetch reached the %s alias for 0x%lx\n", MODE_NAME, (uint64_t)FAULT_ADDR);
        return 1;
    }

    CRITICAL("FAIL: exception did not precisely identify the %s fault\n", MODE_NAME);
    return 2;
}
