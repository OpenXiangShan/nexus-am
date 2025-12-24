#include <klib.h>
#include <nmi.h>


extern void secall_handler_reg(_Context*(*handler)(_Event, _Context*));

#define TAG_ECC_ERROR_INJECTION 1
#define DATA_ECC_ERROR_INJECTION 1

// DCache CtrlUnit reg base addr
#define CTRLUNIT_BASE_ADDR 0x38022000

// OFFSET
#define ECCCTL_OFFSET  0x00  // ECC ctrl reg offset
#define ECCEID_OFFSET  0x08  // ECC delay counter reg offset
#define ECCMASK_OFFSET 0x10  // ECC mask reg offset

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


// Disable timer config
extern int g_config_disable_timer;

uint64_t test_data_array[256];
volatile uint64_t* dat = test_data_array;

// NMI handler counter (must be declared before functions that use it)
static volatile int nmi_count = 0;
static volatile uint64_t last_mnepc = 0;
static volatile uint64_t last_mncause = 0;

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

// wait for injection done
// static void wait_for_injection_complete(uint64_t ctl_addr) {
//     uint64_t ctl_value;
//     do {
//         ctl_value = read_reg(ctl_addr);
//     } while ((ctl_value & (1 << ECCCTL_ESE_BIT)) != 0);
// }

// trigger ECC tag error
void test_tag_ecc_error() {
    // printf("Starting Tag ECC error injection test...\n");

    // make sure dat[i] in dcache
    for(int i=0; i<32; i++) {
      // dat[0];
      dat[i];
    }

    int bank_num = 0;

    // 1. set ECCMASK
    uint64_t tag_mask = 0xFF;  // reverse low 8 bits
    write_reg(CTRLUNIT_BASE_ADDR + ECCMASK_OFFSET + (bank_num * 0x8), tag_mask);
    // printf("  Configured ECCMASK[%d]: 0x%lx\n", bank_num, tag_mask);

    // 2. set ECCEID
    uint64_t delay = 0x32;
    write_reg(CTRLUNIT_BASE_ADDR + ECCEID_OFFSET, delay);
    // printf("  Configured ECCEID: 0x%lx\n", delay);

    // 3. set ECCCTL
    uint64_t ctl_value = 0;
    ctl_value |= (1 << ECCCTL_ESE_BIT);   // ese = 1
    ctl_value |= (0 << ECCCTL_PST_BIT);   // pst = 0
    ctl_value |= (1 << ECCCTL_EDE_BIT);   // ede = 1
    ctl_value |= (0 << ECCCTL_CMP_BIT);   // cmp = 0
    ctl_value |= ((1 << bank_num) << ECCCTL_BANK_BIT);  // bank mask enable

    // printf("  Configured ECCCTL: 0x%lx\n", ctl_value);
    write_reg(CTRLUNIT_BASE_ADDR + ECCCTL_OFFSET, ctl_value);

    // Load
    // Subsequent accesses to the same cache line will trigger more NMIs
    for(int i=0; i<32; i++) {
      dat[i];
    }

    // printf("  Waiting for injection to complete...\n");
    // wait_for_injection_complete(CTRLUNIT_BASE_ADDR + ECCCTL_OFFSET);

    // printf("Tag ECC error injection test completed.\n");
}

// 触发Data ECC错误
void test_data_ecc_error() {
    printf("Starting Data ECC error injection test...\n");

    // make sure dat[i] in DCache
    for(int i=0; i<32; i++) {
      // dat[0];

      dat[i];
    }

    int bank_num = 0;

    // 1. set ECCMASK
    uint64_t data_mask = 0xFFFF0000FFFF0000;
    write_reg(CTRLUNIT_BASE_ADDR + ECCMASK_OFFSET + (bank_num * 0x8), data_mask);
    // printf("  Configured ECCMASK[%d]: 0x%lx\n", bank_num, data_mask);

    // 2. set ECCEID
    uint64_t delay = 0x6;
    write_reg(CTRLUNIT_BASE_ADDR + ECCEID_OFFSET, delay);
    // printf("  Configured ECCEID: 0x%lx\n", delay);

    // 3. set ECCCTL
    uint64_t ctl_value = 0;
    ctl_value |= (1 << ECCCTL_ESE_BIT);   // ese = 1
    ctl_value |= (0 << ECCCTL_PST_BIT);   // pst = 0
    ctl_value |= (1 << ECCCTL_EDE_BIT);   // ede = 1
    ctl_value |= (1 << ECCCTL_CMP_BIT);   // cmp = 1
    ctl_value |= ((1 << bank_num) << ECCCTL_BANK_BIT);  // bank mask enable

    // printf("  Configured ECCCTL: 0x%lx\n", ctl_value);
    write_reg(CTRLUNIT_BASE_ADDR + ECCCTL_OFFSET, ctl_value);


    // trigger interrupt
    for(int i=0; i<32; i++) {
      dat[i];
    }

    // printf("  Waiting for injection to complete...\n");
    // wait_for_injection_complete(CTRLUNIT_BASE_ADDR + ECCCTL_OFFSET);

    // printf("Data ECC error injection test completed.\n");
}

// NMI 处理函数
_Context *nmi_trap_handler(_Event ev, _Context *ctx) {
    nmi_count++;
    last_mnepc = ctx->sepc;     // In _Context, sepc is used for mnepc
    last_mncause = ctx->scause;  // In _Context, scause is used for mncause

    printf("\n=== NMI #%d ===\n", nmi_count);
    printf("mnepc: 0x%llx, mncause: 0x%llx\n", last_mnepc, last_mncause);

    // Read BEU registers BEFORE clearing
    uint64_t beu_cause_before = read_reg(BEU_CAUSE);
    uint64_t beu_accrued_before = read_reg(BEU_ACCRUED_INTR);
    uint64_t beu_local_intr = read_reg(BEU_LOCAL_INTR);

    printf("BEFORE: cause=0x%llx, accrued=0x%llx, local_int=0x%llx\n",
           beu_cause_before, beu_accrued_before, beu_local_intr);

    // to generate a posedge signal for next interrupt in beu
    write_reg(BEU_LOCAL_INTR, 0x0);

    // clear the interrupt source
    write_reg(BEU_ACCRUED_INTR, beu_accrued_before & (~(1 << beu_cause_before)));

    // write 0 to update cause_reg that is the next interrupt cause
    write_reg(BEU_CAUSE, 0x0);

    // Verify clearing worked
    uint64_t beu_accrued_after = read_reg(BEU_ACCRUED_INTR);
    printf("AFTER: accrued=0x%llx\n", beu_accrued_after);

    // 5. Skip the faulting instruction
    if ((*(uint16_t*)last_mnepc & 0x3) == 0x3) {
        // 32-bit instruction
        ctx->sepc += 4;
    } else {
        // 16-bit compressed instruction
        ctx->sepc += 2;
    }

    // 6. Re-enable BEU interrupts after all cleanup, to generate a posedge signal to handler next interrupt
    write_reg(BEU_LOCAL_INTR, beu_local_intr);

    printf("Skipped to mnepc: 0x%llx\n", ctx->sepc);
    printf("===================\n\n");

    return ctx;
}

// Timer interrupt handler (should be disabled but kept for safety)
_Context *timer_trap(_Event ev, _Context *ctx) {
    printf("T");
    return ctx;
}


int main() {
    // 1. Initialize IOE
    _ioe_init();

    // 2. Disable timer interrupt
    g_config_disable_timer = 1;

    // 3. Initialize NMI handler
    _nmi_init(nmi_trap_handler);
    nmi_handler_reg(nmi_trap_handler);

    // 4. Configure BEU interrupt enable
    // printf("Enabling BEU interrupts...\n");
    write_reg(BEU_LOCAL_INTR, 0xff);   // local_interrupt

    // 5. Enable M-mode interrupts (for NMI)
    // Note: NMI doesn't need mie/mstatus settings as it's non-maskable
    // But we set them for completeness
    // printf("Enabling M-mode interrupts...\n");
    // asm volatile("csrs mstatus, %0" : : "r"(0x8));  // MIE bit

    // printf("Setup complete. Starting ECC error injection tests...\n\n");




    // 测试1: Tag Error注入
    #if TAG_ECC_ERROR_INJECTION
    printf("Test 1: Tag ECC Error Injection\n");
    printf("--------------------------------\n");
    test_tag_ecc_error();

    // 等待一段时间确保系统稳定
    for (int i = 0; i < 1000; i++) {
        asm volatile("nop");
    }

    #endif

    #if DATA_ECC_ERROR_INJECTION
    // 测试2: Data Error注入
    printf("Test 2: Data ECC Error Injection\n");
    printf("--------------------------------\n");
    test_data_ecc_error();
    #endif

    return 0;
}
