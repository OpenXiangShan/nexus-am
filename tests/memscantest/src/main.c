//   See LICENSE for license details.

//   the memory space scan test
//   scan all the physical memory address space to see whether physical memory attributes are correct
//           start             end                     description                    priv
//   MemMap("0x00_8000_0000", "0x00_FFFF_FFFF", "h0", "PCIe Slave Space Low PCIe",   "RWX"),
//   MemMap("0x01_0000_0000", "0x07_FFFF_FFFF", "h0", "PCIe Slave Space High PCIe",  "RWX"),
//   MemMap("0x08_0000_0000", "0x1E_FFFF_FFFF", "h0", "Reserved",                    "R"),
//   MemMap("0x1F_0000_0000", "0x1F_0FFF_FFFF", "h0", "CPUSS Perfipheral",           "RW"),
//   MemMap("0x1F_1000_0000", "0x1F_1FFF_FFFF", "h0", "Reserved",                    "R"),
//   MemMap("0x1F_2000_0000", "0x1F_201F_FFFF", "h0", "DDR Config",                  "RW"),
//   MemMap("0x1F_2020_0000", "0x1F_203F_FFFF", "h0", "PCIe PHY",                    "RW"),
//   MemMap("0x1F_2040_0000", "0x1F_2047_FFFF", "h0", "APGC Config",                 "RW"),
//   MemMap("0x1F_2048_0000", "0x1F_2048_FFFF", "h0", "SOC TOP Register",            "RW"),
//   MemMap("0x1F_2049_0000", "0x1F_2049_FFFF", "h0", "DTS",                         "RW"),
//   MemMap("0x1F_204A_0000", "0x1F_204A_FFFF", "h0", "GPIO PAR0",                   "RW"),
//   MemMap("0x1F_204B_0000", "0x1F_204B_FFFF", "h0", "GPIO PAR1",                   "RW"),
//   MemMap("0x1F_204C_0000", "0x1F_204C_FFFF", "h0", "PLL0",                        "RW"),
//   MemMap("0x1F_204D_0000", "0x1F_204D_FFFF", "h0", "PLL1",                        "RW"),
//   MemMap("0x1F_204E_0000", "0x1F_204E_FFFF", "h0", "PLL2",                        "RW"),
//   MemMap("0x1F_204F_0000", "0x1F_204F_03FF", "h0", "Fuse0",                       "RW"),
//   MemMap("0x1F_204F_0400", "0x1F_2049_07FF", "h0", "Fuse1",                       "RW"),
//   MemMap("0x1F_204F_0800", "0x1F_2049_0BFF", "h0", "RTC Register",                "RW"),
//   MemMap("0x1F_204F_0C00", "0x1F_7FFF_FFFF", "h0", "Reserved",                    "R"), // NOTE: not aligned to 4KB
//   MemMap("0x1F_8000_0000", "0x1F_BFFF_FFFF", "h0", "Peripheral SS",               "RWX"),
//   MemMap("0x1F_C000_0000", "0x1F_DFFF_FFFF", "h0", "PCIe Slave Space",            "RW"),
//   MemMap("0x1F_E000_0000", "0x1F_E1FF_FFFF", "h0", "PCI SS Config Space",         "RW"),
//   MemMap("0x1F_E200_0000", "0x1F_E21F_FFFF", "h0", "Shared SRAM",                 "RWX"),
//   MemMap("0x1F_E220_0000", "0x1F_FFF7_FFFF", "h0", "Reserved",                    "R"),
//   MemMap("0x1F_FFF8_0000", "0x1F_FFFB_FFFF", "h0", "BOOT ROM",                    "RWX"),
//   MemMap("0x1F_FFFC_0000", "0x1F_FFFF_FFFF", "h0", "Reserved",                    "R"),
//   MemMap("0x20_0000_0000", "0x23_FFFF_FFFF", "h0", "MEM SS[DDR]",                  "RWX")

// how to run:
// 1. cd memscantest
// 2. make ARCH=riscv64-xs-southlake-flash
// 3. ./emu --i path/to/memscantest-riscv64-xs-southlake-flash.bin  --no-diff  -I 5000000 --force-dump-result  2>temp
// 4. pass if HIT GOOD TRAP appears 

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <test.h>

#define VM_END 0x2400000000UL
#define VM_START 0x80000000UL
#define VM_RANGE 0x00080000

#define error(msg) {printf(msg); _halt(1);}

volatile int trap_expected;
volatile int granule;
volatile int expect_excep;
// for exec test
volatile int total_unexec_segment = 0;
volatile int cur_unexec_id = 0;
volatile int cur_unexec_num = 0;

enum CMD {
  READ, WRITE, EXEC
};

struct MemAttr {
    uintptr_t start;
    char description[512];
    uint8_t read;
    uint8_t write;
    uint8_t exec;
};

struct MemAttr MemAttrs[28] = {
    {0x80000000UL, "PCIe Slave Space Low PCIe", 1, 1, 1},
    {0x100000000UL, "PCIe Slave Space High PCIe", 1, 1, 1},
    {0x800000000UL, "Reserved", 1, 0, 0},
    {0x1f00000000UL, "CPUSS Perfipheral", 1, 1, 0},
    {0x1f10000000UL, "Reserved", 1, 0, 0},
    {0x1f20000000UL, "DDR Config", 1, 1, 0},
    {0x1f20200000UL, "PCIe PHY", 1, 1, 0},
    {0x1f20400000UL, "APGC Config", 1, 1, 0},
    {0x1f20480000UL, "SOC TOP Register", 1, 1, 0},
    {0x1f20490000UL, "DTS", 1, 1, 0},
    {0x1f204a0000UL, "GPIO PAR0", 1, 1, 0},
    {0x1f204b0000UL, "GPIO PAR1", 1, 1, 0},
    {0x1f204c0000UL, "PLL0", 1, 1, 0},
    {0x1f204d0000UL, "PLL1", 1, 1, 0},
    {0x1f204e0000UL, "PLL2", 1, 1, 0},
    {0x1f204f0000UL, "Fuse0", 1, 1, 0},
    {0x1f204f0400UL, "Fuse1", 1, 1, 0},
    {0x1f204f0800UL, "RTC Register", 1, 1, 0},
    {0x1f204f0c00UL, "Reserved", 1, 0, 0},
    {0x1f80000000UL, "Peripheral SS", 1, 1, 1},
    {0x1fc0000000UL, "PCIe Slave Space", 1, 1, 0},
    {0x1fe0000000UL, "PCI SS Config Space", 1, 1, 0},
    {0x1fe2000000UL, "Shared SRAM", 1, 1, 1},
    {0x1fe2200000UL, "Reserved", 1, 0, 0},
    {0x1ffff80000UL, "BOOT ROM", 1, 1, 1},
    {0x1ffffc0000UL, "Reserved", 1, 0, 0},
    {0x2000000000UL, "MEM SS[DDR]", 1, 1, 1},
    {0x2400000000UL, "MEM sentinel(end of DDR)", 0, 0, 0},
};

void exec_test();

static uintptr_t insn_len(uintptr_t pc)
{
  return (*(unsigned short*)pc & 3) ? 4 : 2;
}

int pma_check(uintptr_t address, enum CMD cmd) {
    int idx = 0;
    for(;;idx++) {
        if(address >= MemAttrs[idx].start && address < MemAttrs[idx + 1].start) {
            break;
        }
    }
    if( (cmd == READ && MemAttrs[idx].read) || (cmd == WRITE && MemAttrs[idx].write) || (cmd == EXEC && MemAttrs[idx].exec) ) {
        return 1;
    }
    return 0;
}

// #define INLINE inline __attribute__((always_inline))
#define INLINE __attribute__((noinline))

INLINE void reset_mpp() __attribute__((optimize("O3")));
INLINE void reset_mpp() {
  uintptr_t mpp_s = MSTATUS_MPP;
  asm volatile ("mv t0, %0; csrs mstatus, t0; jr ra" : : "r" (mpp_s));
}

// exception handler
uintptr_t handle_trap(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  if (cause == CAUSE_ILLEGAL_INSTRUCTION) {
    reset_mpp();
    return epc + insn_len(epc);
  }

  if (cause != CAUSE_LOAD_ACCESS && cause != CAUSE_FETCH_ACCESS && cause != CAUSE_STORE_ACCESS)
    error("unexpected exception type(actual exception)\n")
  if (!trap_expected)
    error("operation should not trigger an exception\n")
  if (!(expect_excep == CAUSE_LOAD_ACCESS || expect_excep == CAUSE_FETCH_ACCESS || expect_excep == CAUSE_STORE_ACCESS))
    error("unexpected exception type(expected exception)\n")
  if (cause != expect_excep) {
    error("exception type is not as expected\n")
  }

  trap_expected = 0;
  if (cause == CAUSE_FETCH_ACCESS) {
    cur_unexec_id++;
    cur_unexec_num++;
    return (uintptr_t)exec_test;
  }
  else
    return epc + insn_len(epc);
}

// l1 2G super pages
uintptr_t l1pt[RISCV_PGSIZE / sizeof(uintptr_t)] __attribute__((aligned(RISCV_PGSIZE)));

static void init_pt()
{
    uint64_t HIGHEST_PPN_SHITF = (RISCV_PGSHIFT + RISCV_PGLEVEL_BITS + RISCV_PGLEVEL_BITS);
    // make 2G super page tables between 0x80000000L to 0x2400000000L
    for(uintptr_t start_addr = VM_START; start_addr <= VM_END ; start_addr += (1 << HIGHEST_PPN_SHITF)) {
        l1pt[start_addr >> HIGHEST_PPN_SHITF] = ((uintptr_t)start_addr >> RISCV_PGSHIFT << PTE_PPN_SHIFT) | PTE_V | PTE_X | PTE_R | PTE_W | PTE_A | PTE_D;
    }
  
#if __riscv_xlen == 64
  uintptr_t vm_choice = SATP_MODE_SV39;
#else
  uintptr_t vm_choice = SATP_MODE_SV32;
#endif
  write_csr(sptbr, ((uintptr_t)l1pt >> RISCV_PGSHIFT) |
                   (vm_choice * (SATP_MODE & ~(SATP_MODE<<1))));
}

INLINE void test_one_st(uintptr_t addr, uintptr_t size)
{
  uintptr_t new_mstatus = (read_csr(mstatus) & ~MSTATUS_MPP) | (MSTATUS_MPP & (MSTATUS_MPP >> 1)) | MSTATUS_MPRV;
  switch (size) {
    case 1: asm volatile ("csrrw %0, mstatus, %0; sb x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
    case 2: asm volatile ("csrrw %0, mstatus, %0; sh x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
    case 4: asm volatile ("csrrw %0, mstatus, %0; sw x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
#if __riscv_xlen >= 64
    case 8: asm volatile ("csrrw %0, mstatus, %0; sd x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
#endif
    default: __builtin_unreachable();
  }
}

INLINE void test_one_ld(uintptr_t addr, uintptr_t size)
{
  uintptr_t new_mstatus = (read_csr(mstatus) & ~MSTATUS_MPP) | (MSTATUS_MPP & (MSTATUS_MPP >> 1)) | MSTATUS_MPRV;
  switch (size) {
    case 1: asm volatile ("csrrw %0, mstatus, %0; lb x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
    case 2: asm volatile ("csrrw %0, mstatus, %0; lh x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
    case 4: asm volatile ("csrrw %0, mstatus, %0; lw x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
#if __riscv_xlen >= 64
    case 8: asm volatile ("csrrw %0, mstatus, %0; ld x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
#endif
    default: __builtin_unreachable();
  }
}

INLINE void test_one_ldst(uintptr_t addr, int size) {
  expect_excep = CAUSE_LOAD_ACCESS;
  trap_expected = !pma_check(addr, READ);
  test_one_ld(addr, size);
  if (trap_expected)
    error("exception should be handled correctly or should not be triggered\n")

  expect_excep = CAUSE_STORE_ACCESS;
  trap_expected = !pma_check(addr, WRITE);
  test_one_st(addr, size);
  if (trap_expected)
    error("exception should be handled correctly or should not be triggered\n")
}

INLINE void test_all_sizes(uintptr_t addr)
{
  for (size_t size = 1; size <= sizeof(uintptr_t); size *= 2) {
    if (addr & (size - 1))
      continue;
    test_one_ldst(addr, size);
  }
}

INLINE void test_range_once(uintptr_t base, uintptr_t range)
{
  for (uintptr_t addr = base; addr < base + range; addr += granule)
    test_all_sizes(addr);
}

INLINE void nothing() __attribute__((optimize("O0")));
INLINE void nothing() {
  return ;
}

INLINE void turn_to_smode() __attribute__((optimize("O3")));
INLINE void turn_to_smode() {
  uintptr_t mpp_s = MSTATUS_MPP & (MSTATUS_MPP >> 1);
  asm volatile ("mv t0, %0; csrs mstatus, t0; csrw mepc, ra; mret" : : "r" (mpp_s));
}

void get_all_unexec_segments() {
    for(int i=0;i<27;i++) {
        if(MemAttrs[i].exec == 0)
            total_unexec_segment++;
    }
}

void exec_test() {
    for(;cur_unexec_id<27;) {
        if(MemAttrs[cur_unexec_id].exec == 0) {
          expect_excep = CAUSE_FETCH_ACCESS;
          trap_expected = 1;
          void(*ptr)();
          ptr = (void(*)())(MemAttrs[cur_unexec_id].start);
          ptr();
        }else {
          trap_expected = 0;
          cur_unexec_id++;
        }
    }
    if(total_unexec_segment == cur_unexec_num) {
      _halt(0);
    }
}

void naive_rw_mem_scan_test(uintptr_t range) {
  test_range_once(0x80000000UL, range);
  test_range_once(0x100000000UL, range);
  test_range_once(0x800000000UL, range);
  test_range_once(0x1F00000000UL, range);
  // test_range_once(0x1F10000000UL, range); // error
  test_range_once(0x1F20000000UL, range);
  test_range_once(0x1F20200000UL, range);
  test_range_once(0x1F20400000UL, range);
  test_range_once(0x1F20480000UL, range);
  test_range_once(0x1F20490000UL, range);
  test_range_once(0x1F204A0000UL, range);
  test_range_once(0x1F204B0000UL, range);
  test_range_once(0x1F204C0000UL, range);
  test_range_once(0x1F204D0000UL, range);
  test_range_once(0x1F204E0000UL, range);
  test_range_once(0x1F204F0000UL, range);
  test_range_once(0x1F80000000UL, range);
  test_range_once(0x1FC0000000UL, range);
  test_range_once(0x1FE0000000UL, range);
  test_range_once(0x1FE2000000UL, range);
  test_range_once(0x1FE2200000UL, range); 
  test_range_once(0x1FFFF80000UL, range);
  test_range_once(0x1FFFFC0000UL, range);
}

void init() {
  extern void trap_entry(void);
  // register M mode exception handler
  asm volatile("csrw mtvec, %0" : : "r"(trap_entry));
}

int main() __attribute__((optimize("O0")));
int main()
{
  // note: this test is under M mode
  init();
  init_pmp();
  granule = 8;
  init_pt();

  naive_rw_mem_scan_test(8);
  uintptr_t mprv = MSTATUS_MPRV;
  asm volatile ("csrc mstatus, %0" : : "r" (mprv));
  get_all_unexec_segments();
  exec_test();

  return 0;
}
