#include <klib.h>
#include <nmi.h>

#define LOG_ENABLED 0
#define FOR_NUM 16

#define TAG_ECC_ERROR_INJECTION 1
#define DATA_ECC_ERROR_INJECTION 1

// DCache CtrlUnit reg base addr
#define CTRLUNIT_BASE_ADDR 0x38022000

// OFFSET
#define ECCCTL_OFFSET  0x00  // ECC ctrl reg offset
#define ECCEID_OFFSET  0x08  // ECC delay counter reg offset
#define ECCMASK_OFFSET 0x10  // ECC mask reg offset
#define ECCADDRSTART_OFFSET 0x50  // ECC addr start reg offset
#define ECCADDREND_OFFSET 0x58  // ECC addr end reg offset

// ECCCTL reg every bit define
#define ECCCTL_ESE_BIT 0     // error signaling enable
#define ECCCTL_PST_BIT 1     // persistent injection, not use, we just trigger once ecc error
#define ECCCTL_EDE_BIT 2     // error delay enable
#define ECCCTL_CMP_BIT 3     // component (0: tag, 1: data)
#define ECCCTL_BANK_BIT 4    // bank enable, 8 bit, every bit enable one mask, 0b0000_0010 means enable mask1

// BEU interrupt addr
#define BEU_CAUSE         0x38010000
#define BEU_VALUE         0x38010008
#define BEU_ENABLE        0x38010010
#define BEU_GLOBAL_INTR   0x38010018
#define BEU_ACCRUED_INTR  0x38010020
#define BEU_LOCAL_INTR    0x38010028

// banks of DCache cacheline are not confirm
// kmhv2 use BANK_8
// kmhv3 use BANK_32
// only support BANK_8 now. The code design has not yet been finalized for implementation in kmhv3.
#define BANK_8

#if defined (BANK_8)
#define TEST_TYPE uint64_t
#elif defined (BANK_32)
#define TEST_TYPE uint16_t
#endif

// Disable timer config
extern int g_config_disable_timer;

extern void secall_handler_reg(_Context*(*handler)(_Event, _Context*));

// Test data uses a dedicated section and 4 KB alignment to avoid sharing a cache set with critical variables.
TEST_TYPE test_data_array[1024] __attribute__((section(".ecc_test_data"), aligned(4096))) = {0};

uint64_t save_beu_value[32] = {0};
uint64_t save_mnepc[32] = {0};

// NMI handler counter
static volatile int last_nmi_cnt = 0;
static volatile int nmi_count = 0;
static volatile uint64_t last_mnepc = 0;
static volatile uint64_t last_mncause = 0;
static volatile uint64_t last_mnstatus = 0;
static volatile uint64_t last_mnscratch = 0;
static volatile uint64_t last_beu_cause = 0;
static volatile uint64_t last_beu_accrued = 0;
static volatile uint64_t last_beu_local_intr = 0;
static volatile uint64_t last_beu_value = 0;

static int exception = 0;

void my_printf(const char *fmt, ...) {
#if LOG_ENABLED
    // Simple wrapper around vprintf to handle variable arguments
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#endif
}

// mmio read/write
static inline void write_reg(uint64_t addr, uint64_t value) {
    volatile uint64_t *reg = (volatile uint64_t *)addr;
    *reg = value;
    asm volatile("fence iorw, iorw" ::: "memory");
}

static inline uint64_t read_reg(uint64_t addr) {
    volatile uint64_t *reg = (volatile uint64_t *)addr;
    asm volatile("fence iorw, iorw" ::: "memory");
    return *reg;
}

// trigger ECC tag error
void test_tag_ecc_error(int index) {
    my_printf("Starting Tag ECC error injection test...\n");
    volatile TEST_TYPE *target = &test_data_array[index];

    int bank_num = 0;

    if(index == 0) {
      write_reg(CTRLUNIT_BASE_ADDR + ECCADDRSTART_OFFSET, 0x80002000);
      write_reg(CTRLUNIT_BASE_ADDR + ECCADDREND_OFFSET, 0x80004000);
  
      // 1. set ECCMASK
      uint64_t tag_mask = 0x3f3f3f3f3f3f3f3f;  // reverse low 8 bits
      write_reg(CTRLUNIT_BASE_ADDR + ECCMASK_OFFSET + (bank_num * 0x8), tag_mask);
      my_printf("  Configured ECCMASK[%d]: 0x%lx\n", bank_num, tag_mask);
  
      // 2. set ECCEID
      uint64_t delay = 0x6;
      write_reg(CTRLUNIT_BASE_ADDR + ECCEID_OFFSET, delay);
      my_printf("  Configured ECCEID: 0x%lx\n", delay);
    }

    // 3. set ECCCTL
    uint64_t ctl_value = 0;
    ctl_value |= (1 << ECCCTL_ESE_BIT);   // ese = 1
    ctl_value |= (0 << ECCCTL_PST_BIT);   // pst = 0
    ctl_value |= (1 << ECCCTL_EDE_BIT);   // ede = 1
    ctl_value |= (0 << ECCCTL_CMP_BIT);   // cmp = 0
    ctl_value |= ((1 << bank_num) << ECCCTL_BANK_BIT);  // bank mask enable

    my_printf("  Configured ECCCTL: 0x%lx\n", ctl_value);
    write_reg(CTRLUNIT_BASE_ADDR + ECCCTL_OFFSET, ctl_value);

    // trigger ECC error by accessing the target address multiple times
    do {
        asm volatile(
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "ld t0, 0(%0)\n\t"
            "fence\n\t"
            : : "r"(target) : "t0", "memory"
        );

        my_printf("save_mnepc[%d] = 0x%llx, save_beu_value[%d] = 0x%llx\n", last_nmi_cnt, save_mnepc[last_nmi_cnt], last_nmi_cnt, save_beu_value[last_nmi_cnt]);

    } while(0);
}

// Trigger a data ECC error with an unrolled loop to avoid stack accesses from loop variables.
void test_data_ecc_error(int index) {
    volatile TEST_TYPE *target = &test_data_array[index];  // Access the array element directly.

    int bank_num = index;

    write_reg(CTRLUNIT_BASE_ADDR + ECCADDRSTART_OFFSET, 0x80002000);
    write_reg(CTRLUNIT_BASE_ADDR + ECCADDREND_OFFSET, 0x80004000);

    // 1. set ECCMASK
    uint64_t data_mask = 0x3f3f3f3f3f3f3f3f; // hit ecc error
    write_reg(CTRLUNIT_BASE_ADDR + ECCMASK_OFFSET + (bank_num * 0x8), data_mask);
    
    if(index == 0) {
      // 2. set ECCEID
      write_reg(CTRLUNIT_BASE_ADDR + ECCEID_OFFSET, 0x6);
    }

    // 3. set ECCCTL
    uint64_t ctl_value = (1 << ECCCTL_ESE_BIT) | 
                         (0 << ECCCTL_PST_BIT) |
                         (1 << ECCCTL_EDE_BIT) |
                         (1 << ECCCTL_CMP_BIT) |
                         ((1 << bank_num) << ECCCTL_BANK_BIT);
    write_reg(CTRLUNIT_BASE_ADDR + ECCCTL_OFFSET, ctl_value);

    // 4. Trigger immediately with unrolled inline assembly to avoid loop and stack accesses.
    asm volatile(
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "ld t0, 0(%0)\n\t"
      "fence\n\t"
      : : "r"(target) : "t0", "memory"
    );
}

// NMI handler
_Context *nmi_trap_handler(_Event ev, _Context *ctx) {
    asm volatile("fence iorw, iorw" ::: "memory");

    last_nmi_cnt = nmi_count++;


    last_mnepc = ctx->sepc;     // In _Context, sepc is used for mnepc
    last_mncause = ctx->scause;  // In _Context, scause is used for mncause

    asm volatile("csrr %0, mnstatus" : "=r" (last_mnstatus));
    asm volatile("csrr %0, mnscratch" : "=r" (last_mnscratch));

    // Read BEU registers BEFORE clearing
    last_beu_cause = read_reg(BEU_CAUSE);
    last_beu_accrued = read_reg(BEU_ACCRUED_INTR);
    last_beu_local_intr = read_reg(BEU_LOCAL_INTR);
    last_beu_value = read_reg(BEU_VALUE);

    save_beu_value[last_nmi_cnt] = last_beu_value;
    save_mnepc[last_nmi_cnt] = last_mnepc;

    // to generate a posedge signal for next interrupt in beu
    write_reg(BEU_LOCAL_INTR, 0x0);

    // clear the interrupt source
    write_reg(BEU_ACCRUED_INTR, last_beu_accrued & (~(1 << last_beu_cause)));

    // write 0 to update cause_reg that is the next interrupt cause
    write_reg(BEU_CAUSE, 0x0);


    // 6. Re-enable BEU interrupts after all cleanup, to generate a posedge signal to handler next interrupt
    write_reg(BEU_LOCAL_INTR, last_beu_local_intr);


    if((last_mncause & 0xff) != 0x1f || (last_mnstatus != 0x1800)) {
        exception = 1;
    }
    asm volatile("fence iorw, iorw" ::: "memory");

    return ctx;
}

// Timer interrupt handler (should be disabled but kept for safety)
_Context *timer_trap(_Event ev, _Context *ctx) {
    my_printf("T");
    return ctx;
}


int main() {
    // Initialize IOE
    // _ioe_init();

    // Disable timer interrupt
    g_config_disable_timer = 1;

    // Initialize NMI handler
    _nmi_init(nmi_trap_handler);

    // Configure BEU interrupt enable
    write_reg(BEU_LOCAL_INTR, 0xff);   // local_interrupt

    // make sure test_data_array[i] in DCache
    for(int i=0; i<sizeof(test_data_array)/sizeof(test_data_array[0]); i++) {
      int tmp = (int)test_data_array[i];
      tmp = ~tmp;
    }

    // Test 1: Tag error injection
    #if TAG_ECC_ERROR_INJECTION
    my_printf("Test 1: Tag ECC Error Injection\n\n");

    for(int i=0; i<8; i++) {
        test_tag_ecc_error(i);
    }

    // Wait briefly for the system to stabilize.
    for (int i = 0; i < 1000; i++) {
        asm volatile("nop");
    }
    #endif

    #if DATA_ECC_ERROR_INJECTION
    // Test 2: Data error injection
    my_printf("Test 2: Data ECC Error Injection\n\n");

    for(int i=0; i<8; i++) {
      my_printf("Injecting Data ECC error in bank %d\n", i);
      test_data_ecc_error(i);
      my_printf("\n=== NMI #0x%llx in %dth for_iter ===\n", nmi_count, i);
      my_printf("mnepc: 0x%llx, mncause: 0x%llx, mnstatus: 0x%llx, mnscratch: 0x%llx\n",
      last_mnepc, last_mncause, last_mnstatus, last_mnscratch);
      my_printf("beu_cause: 0x%llx, beu_accrued: 0x%llx, beu_local_intr: 0x%llx, beu_value: 0x%llx\n",
      last_beu_cause, last_beu_accrued, last_beu_local_intr, last_beu_value);
    }
    #endif

    printf("test done, total %d handler trigger\n", nmi_count);
    for(int i=0; i<nmi_count; i++) {
      printf("save_mnepc[%d] = 0x%llx, save_beu_value[%d] = 0x%llx, bank: 0x%llx\n", i, save_mnepc[i], i, save_beu_value[i], (save_beu_value[i] & 0x38ll) >> 3);
    }

    return 0;
}
