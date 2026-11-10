#include <klib.h>

#define TARGET_VA       0x8020100cUL
#define VS_L0_GPA       0x8003f000UL
#define TARGET_PTE_GPA  (VS_L0_GPA + 8)
#define EXPECTED_MTVAL2 (TARGET_PTE_GPA >> 2)

#define G_ROOT_PA       0x81000000UL
#define G_L1_PA         0x81004000UL
#define VS_ROOT_PA      0x81010000UL
#define VS_L1_PA        0x81011000UL
#define VS_L0_PA        0x81012000UL
#define CODE_PA         0x81020000UL

#define PTE_V           (1UL << 0)
#define PTE_R           (1UL << 1)
#define PTE_W           (1UL << 2)
#define PTE_X           (1UL << 3)
#define PTE_U           (1UL << 4)
#define PTE_A           (1UL << 6)
#define PTE_D           (1UL << 7)
#define PTE_LEAF        (PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D)
#define PTE_G_LEAF      (PTE_LEAF | PTE_U)

#define HSTATUS_VSXL_MASK (3UL << 32)
#define HSTATUS_VSXL_64   (2UL << 32)
#define MISA_H            (1UL << ('H' - 'A'))

#define SATP_SV39        (8UL << 60)
#define HGATP_SV39       (8UL << 60)

#define MCAUSE_INST_GUEST_PAGE_FAULT 20UL
#define MTINST_IMPLICIT_READ          0x3000UL

_Static_assert((G_ROOT_PA & 0x3fffUL) == 0,
               "Sv39x4 root must be 16 KiB aligned");
_Static_assert((TARGET_VA >> 12) & 1UL,
               "the target must select VS L0 PTE index 1");

#define READ_CSR(csr)                                             \
    ({                                                            \
        uint64_t value;                                           \
        asm volatile("csrr %0, " #csr : "=r"(value));             \
        value;                                                    \
    })

#define WRITE_CSR(csr, value)                                     \
    do {                                                          \
        uint64_t csr_value = (uint64_t)(value);                   \
        asm volatile("csrw " #csr ", %0" :: "r"(csr_value)        \
                     : "memory");                                 \
    } while (0)

volatile uint64_t trap_cause;
volatile uint64_t trap_epc;
volatile uint64_t trap_tval;
volatile uint64_t trap_tval2;
volatile uint64_t trap_tinst;
volatile uint64_t trap_resume;
volatile uint32_t trap_count;

extern char trap_handler[];

__asm__(
".globl trap_handler\n"
".align 2\n"
"trap_handler:\n"
"  csrr t0, mcause\n"
"  la   t1, trap_cause\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, mepc\n"
"  la   t1, trap_epc\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, mtval\n"
"  la   t1, trap_tval\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, 0x34b\n"             /* mtval2 */
"  la   t1, trap_tval2\n"
"  sd   t0, 0(t1)\n"
"  csrr t0, 0x34a\n"             /* mtinst */
"  la   t1, trap_tinst\n"
"  sd   t0, 0(t1)\n"
"  la   t1, trap_count\n"
"  lw   t0, 0(t1)\n"
"  addi t0, t0, 1\n"
"  sw   t0, 0(t1)\n"
"  la   t1, trap_resume\n"
"  ld   t0, 0(t1)\n"
"  csrw mepc, t0\n"
"  li   t0, 0x1800\n"             /* MPP=M, MPV=0 */
"  csrw mstatus, t0\n"
"  mret\n"
);

static inline uint64_t pte_table(uint64_t pa) {
    return (pa >> 2) | PTE_V;
}

static inline uint64_t pte_leaf(uint64_t pa, uint64_t flags) {
    return (pa >> 2) | flags;
}

static void clear_entries(uint64_t pa, uint32_t entries) {
    volatile uint64_t *table = (volatile uint64_t *)pa;
    for (uint32_t i = 0; i < entries; i++) {
        table[i] = 0;
    }
}

static int enable_h_extension(void) {
    if ((READ_CSR(0x301) & MISA_H) == 0) {
        return 0;
    }

    uint64_t hstatus = READ_CSR(0x600);
    hstatus = (hstatus & ~HSTATUS_VSXL_MASK) | HSTATUS_VSXL_64;
    WRITE_CSR(0x600, hstatus);
    return (READ_CSR(0x600) & HSTATUS_VSXL_MASK) == HSTATUS_VSXL_64;
}

static void build_page_tables(void) {
    clear_entries(G_ROOT_PA, 2048);
    clear_entries(G_L1_PA, 512);
    clear_entries(VS_ROOT_PA, 512);
    clear_entries(VS_L1_PA, 512);
    clear_entries(VS_L0_PA, 512);

    volatile uint64_t *g_root  = (volatile uint64_t *)G_ROOT_PA;
    volatile uint64_t *g_l1    = (volatile uint64_t *)G_L1_PA;
    volatile uint64_t *vs_root = (volatile uint64_t *)VS_ROOT_PA;
    volatile uint64_t *vs_l1   = (volatile uint64_t *)VS_L1_PA;
    volatile uint64_t *vs_l0   = (volatile uint64_t *)VS_L0_PA;

    /* GPA 0x8003f008 and the test tables share this G-stage root entry. */
    g_root[2] = pte_table(G_L1_PA);
    /* Identity-map the test window; g_l1[0] covering 0x8003f008 stays invalid. */
    g_l1[8] = pte_leaf(0x81000000UL, PTE_G_LEAF);

    vs_root[2] = pte_table(VS_L1_PA);
    vs_l1[1]   = pte_table(VS_L0_GPA);
    vs_l0[1]   = pte_leaf(CODE_PA, PTE_LEAF);

    asm volatile("fence rw, rw; fence.i" ::: "memory");
}

static void prepare_target_code(void) {
    volatile uint32_t *code = (volatile uint32_t *)CODE_PA;
    for (uint32_t i = 0; i < 1024; i++) {
        code[i] = 0x00000013U; /* nop */
    }
    asm volatile("fence rw, rw; fence.i" ::: "memory");
}

static void enter_vs(uint64_t entry) __attribute__((noinline));
static void enter_vs(uint64_t entry) {
    asm volatile(
        "la   t0, 1f\n\t"
        "la   t1, trap_resume\n\t"
        "sd   t0, 0(t1)\n\t"
        "csrw mepc, %0\n\t"
        "li   t0, 0x8000000800\n\t" /* MPP=S, MPV=1 */
        "csrw mstatus, t0\n\t"
        "mret\n\t"
        "1:\n\t"
        :: "r"(entry) : "t0", "t1", "memory");
}

int main(void) {
    if (!enable_h_extension()) {
        printf("UNSUPPORTED: H extension or VSXL=64 is required\n");
        return 0;
    }

    WRITE_CSR(0x180, 0);
    WRITE_CSR(0x302, 0);
    WRITE_CSR(0x303, 0);
    WRITE_CSR(0x3b0, -1UL);
    WRITE_CSR(0x3a0, 0x1fUL);

    build_page_tables();
    prepare_target_code();

    trap_cause = trap_epc = trap_tval = trap_tval2 = trap_tinst = 0;
    trap_resume = 0;
    trap_count = 0;
    WRITE_CSR(0x305, (uint64_t)trap_handler);

    WRITE_CSR(0x280, SATP_SV39 | (VS_ROOT_PA >> 12));
    WRITE_CSR(0x680, HGATP_SV39 | (G_ROOT_PA >> 12));
    asm volatile(".word 0x22000073" ::: "memory"); /* hfence.vvma */
    asm volatile(".word 0x62000073" ::: "memory"); /* hfence.gvma */
    asm volatile("fence.i" ::: "memory");

    printf("Entering VS at 0x%lx, expected mtval2=0x%lx\n",
           TARGET_VA, EXPECTED_MTVAL2);
    enter_vs(TARGET_VA);

    printf("Caught traps=%u mcause=0x%lx mepc=0x%lx mtval=0x%lx "
           "mtval2=0x%lx mtinst=0x%lx\n",
           trap_count, trap_cause, trap_epc, trap_tval,
           trap_tval2, trap_tinst);

    int pass = trap_count == 1 &&
               trap_cause == MCAUSE_INST_GUEST_PAGE_FAULT &&
               trap_epc == TARGET_VA &&
               (trap_tval == 0 || trap_tval == TARGET_VA) &&
               trap_tval2 == EXPECTED_MTVAL2 &&
               trap_tinst == MTINST_IMPLICIT_READ;

    if (pass) {
        printf("PASS\n");
        return 0;
    }

    printf("FAIL: expected one implicit VS PTE guest page fault with "
           "mtval2=0x%lx\n", EXPECTED_MTVAL2);
    return 1;
}
