// Test fetch block crossing page boundary with different page attributes.

// TESTMODE: yx
// x: single 32-bit RVI instruction crosses page boundary
//    expected mepc=0x82fffffe
// 1x: full RVC instructions, fetch block crosses page boundary
//    expected mepc=0x83000000

// y1: crossing EXEC+IO -> IO
//     expected mcause=0xc
// y2: crossing EXEC+IO -> EXEC
//     expected mcause=0x1
// y3: crossing EXEC -> EXEC+IO
//     expected mcause=0x1

// NOTE: TEST_MODE 12 expected NO TRAP,
// because we can fetch 0x82fffffe->0x83000000 as an RVC instruction, then fetch the next fetch block,
// so actually no cross-page fetch happens in TEST_MODE 12, and the test is expected to pass without exception.

// NOTE: TEST_MODE y2/y3 expected to cause difftest failure, as NEMU will allow cross-page fetch with different page attributes, while XiangShan will not.

#ifndef TEST_MODE
#define TEST_MODE 1
#endif

#if !((TEST_MODE == 1) || (TEST_MODE == 2) || (TEST_MODE == 3) || \
    (TEST_MODE == 11) || (TEST_MODE == 12) || (TEST_MODE == 13))
#error "TEST_MODE must be one of {1,2,3,11,12,13}"
#endif

#include <klib.h>

extern void init_pmp(void);

extern unsigned char test_func_start[];
extern unsigned char test_func_end[];

// Mode 1/2/3: single 32-bit RVI instruction crosses page boundary (start @ 0x82fffffa)
// Mode 11/12/13: RVC instruction block crosses page boundary (start @ 0x82fffff8)
#if (TEST_MODE == 1) || (TEST_MODE == 2) || (TEST_MODE == 3)
asm(
    ".pushsection .crosspage_test, \"ax\", @progbits\n"
    ".balign 2\n"
    ".option push\n"
    ".option norvc\n"
    "  .2byte 0x0001\n"
    ".global test_func_start\n"
    "test_func_start:\n"
    "  lui t3, 0x12345\n"
    "  xori t3, t3, 0x7f\n"
    "  addi a0, t3, 0\n"
    "  jalr x0, 0(ra)\n"
    ".global test_func_end\n"
    "test_func_end:\n"
    ".option pop\n"
    ".popsection\n"
);
#else
asm(
    ".pushsection .crosspage_test, \"ax\", @progbits\n"
    ".balign 4\n"
    ".option push\n"
    ".option rvc\n"
    ".global test_func_start\n"
    "test_func_start:\n"
    "  c.li a0, 0\n"
    "  .rept 16\n"
    "    c.addi a0, 1\n"
    "  .endr\n"
    "  c.jr ra\n"
    ".global test_func_end\n"
    "test_func_end:\n"
    ".option pop\n"
    ".popsection\n"
);
#endif

typedef uint64_t pte_t;

#define SATP_MODE_SV39 (8ull << 60)
#define SATP_PPN_MASK  0x00000fffffffffffULL
#define PTE_PBMT_IO    (2ull << 61)
#define ENVCFG_PBMTE   (1ull << 62)

#define PTE_V 0x001ull
#define PTE_R 0x002ull
#define PTE_W 0x004ull
#define PTE_X 0x008ull
#define PTE_A 0x040ull
#define PTE_D 0x080ull
#define PTE_ADDR(pte)   (((uintptr_t)(pte) & ~0x3ffull) << 2)

#define PT_ENTRIES 512
#define PAGE_SIZE  4096

static pte_t page_tables[8][PT_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uintptr_t next_pt = 1;

#if __riscv_xlen == 64
#define REG_STORE "sd"
#else
#define REG_STORE "sw"
#endif

__attribute__((aligned(4))) void trap_entry(void) {

    uintptr_t trap_mcause = 0;
    uintptr_t trap_mepc = 0;
    uintptr_t trap_mtval = 0;

    asm volatile("csrr %0, mcause" : "=r"(trap_mcause));
    asm volatile("csrr %0, mepc" : "=r"(trap_mepc));
    asm volatile("csrr %0, mtval" : "=r"(trap_mtval));

    printf("trap: mepc=0x%lx mtval=0x%lx mcause=0x%lx\n",
           (unsigned long)trap_mepc,
           (unsigned long)trap_mtval,
           (unsigned long)trap_mcause);

    _halt(0);
}

#if (TEST_MODE == 1) || (TEST_MODE == 2) || (TEST_MODE == 3)
static const uintptr_t func_ptr = 0x82fffffaull;
#else
static const uintptr_t func_ptr = 0x82fffff8ull;
#endif
static const uintptr_t page0 = 0x82fff000ull;
static const uintptr_t page1 = 0x83000000ull;

#if (TEST_MODE == 1) || (TEST_MODE == 11)
static const pte_t page0_flags = (pte_t)(PTE_V | PTE_R | PTE_X | PTE_A | PTE_D) | PTE_PBMT_IO;
static const pte_t page1_flags = (pte_t)(PTE_V | PTE_R | PTE_A | PTE_D) | PTE_PBMT_IO;
#elif (TEST_MODE == 2) || (TEST_MODE == 12)
static const pte_t page0_flags = (pte_t)(PTE_V | PTE_R | PTE_X | PTE_A | PTE_D) | PTE_PBMT_IO;
static const pte_t page1_flags = (pte_t)(PTE_V | PTE_R | PTE_X | PTE_A | PTE_D);
#else
static const pte_t page0_flags = (pte_t)(PTE_V | PTE_R | PTE_X | PTE_A | PTE_D);
static const pte_t page1_flags = (pte_t)(PTE_V | PTE_R | PTE_X | PTE_A | PTE_D) | PTE_PBMT_IO;
#endif

static inline uintptr_t vpn2(uintptr_t va) {
    return (va >> 30) & 0x1ff;
}

static inline uintptr_t vpn1(uintptr_t va) {
    return (va >> 21) & 0x1ff;
}

static inline uintptr_t vpn0(uintptr_t va) {
    return (va >> 12) & 0x1ff;
}

static inline pte_t pte_next_table(uintptr_t pa) {
    return ((pa >> 12) << 10) | PTE_V;
}

static inline pte_t pte_leaf(uintptr_t pa, pte_t flags) {
    return ((pa >> 12) << 10) | flags;
}

static pte_t *alloc_pt_page(void) {
    pte_t *pt = page_tables[next_pt++];
    uintptr_t i = 0;
    for (i = 0; i < PT_ENTRIES; i++) {
        pt[i] = 0;
    }
    return pt;
}

static void map_4k_page(uintptr_t va, uintptr_t pa, pte_t flags) {
    pte_t *l1 = page_tables[0];
    pte_t *l2 = 0;
    pte_t *l3 = 0;
    uintptr_t i2 = vpn2(va);
    uintptr_t i1 = vpn1(va);
    uintptr_t i0 = vpn0(va);

    if ((l1[i2] & PTE_V) == 0) {
        pte_t *new_l2 = alloc_pt_page();
        l1[i2] = pte_next_table((uintptr_t)new_l2);
    }
    l2 = (pte_t *)PTE_ADDR(l1[i2]);

    if ((l2[i1] & PTE_V) == 0) {
        pte_t *new_l3 = alloc_pt_page();
        l2[i1] = pte_next_table((uintptr_t)new_l3);
    }
    l3 = (pte_t *)PTE_ADDR(l2[i1]);
    l3[i0] = pte_leaf(pa, flags);
}

static void setup_sv39_svpbmt(void) {
    uintptr_t i = 0;
    for (i = 0; i < PT_ENTRIES; i++) {
        page_tables[0][i] = 0;
    }
    next_pt = 1;

    map_4k_page(page0, page0, page0_flags);
    map_4k_page(page1, page1, page1_flags);
}

static void enter_smode(uintptr_t entry) {
    uintptr_t mstatus = 0;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus = (mstatus & ~(3ull << 11)) | (1ull << 11);
    asm volatile("csrw mstatus, %0" :: "r"(mstatus));
    asm volatile("csrw mepc, %0" :: "r"(entry));
    asm volatile("mret");

    while (1) {
    }
}

int main() {
    uintptr_t mtvec_base = ((uintptr_t)trap_entry) & ~0x3ull;

    if ((uintptr_t)test_func_start != func_ptr) {
        _halt(2);
    }

    printf("TEST_MODE=%d: func_ptr=0x%lx\n", TEST_MODE, (unsigned long)func_ptr);

    asm volatile("csrw mtvec, %0" :: "r"(mtvec_base));

    init_pmp();

    asm volatile("csrw medeleg, zero");
    asm volatile("csrw mideleg, zero");

    setup_sv39_svpbmt();

    asm volatile("csrs 0x30a, %0" :: "r"(ENVCFG_PBMTE));
    asm volatile("csrs 0x10a, %0" :: "r"(ENVCFG_PBMTE));

    uintptr_t satp_val = SATP_MODE_SV39 | ((((uintptr_t)page_tables[0]) >> 12) & SATP_PPN_MASK);
    asm volatile("csrw satp, %0" :: "r"(satp_val));
    asm volatile("sfence.vma zero, zero");

    enter_smode(func_ptr);

    return 0;
}
