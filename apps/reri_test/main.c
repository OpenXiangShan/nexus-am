#include <am.h>
#include <klib.h>
#include <nmi.h>
#include <stdint.h>

#define RERI_BASE          0x38010000UL
#define RERI_HEADER_BYTES  0x40UL
#define RERI_RECORD_BYTES  0x40UL
#define RERI_CTRL          0x00UL
#define RERI_STATUS        0x08UL
#define RERI_ADDRESS       0x10UL
#define RERI_BANK_INFO     0x08UL
#define RERI_VALID_SUMMARY 0x10UL

#define RERI_V             (1ULL << 0)
#define RERI_CE            (1ULL << 1)
#define RERI_UEC           (1ULL << 3)
#define RERI_CECO          (1ULL << 21)
#define RERI_RDIP          (1ULL << 23)

#define RERI_ELSE          (1ULL << 0)
#define RERI_CECE          (1ULL << 1)
#define RERI_UECS_SHIFT    6U
#define RERI_EID_SHIFT     32U
#define RERI_SINV          (1ULL << 48)
#define RERI_SRDP          (1ULL << 49)

#define RERI_RECORD(i)     (RERI_BASE + RERI_HEADER_BYTES + (i) * RERI_RECORD_BYTES)

#define RERI_RECORDS       8U
#define RERI_SOFTWARE_RECORD 5U
#define RERI_EXPECTED_VENDOR 0x86fULL
#define RERI_EXPECTED_VERSION 1ULL
#define RERI_SUMMARY_SV      (1ULL << 0)
#ifndef RERI_INJECTIONS
#define RERI_INJECTIONS    100U
#endif
#ifndef RERI_PROGRESS_STEP
#define RERI_PROGRESS_STEP 10U
#endif
#if RERI_INJECTIONS < 4
#error RERI_INJECTIONS must cover all four injection kinds
#endif
#if RERI_PROGRESS_STEP < 1
#error RERI_PROGRESS_STEP must be positive
#endif

#define RERI_NMI_CAUSE     ((1ULL << 63) | 31ULL)
#define MNSTATUS_NMIE     (1ULL << 3)
#define RERI_UECS_MASK    (3ULL << RERI_UECS_SHIFT)
#define MAX_WAIT           20000U
#define NMI_SETTLE_CYCLES  1024U

enum TestAddressRegion {
  TEST_ADDR_RECORD = 1,
  TEST_ADDR_EID    = 2,
  TEST_ADDR_NMI    = 3,
};

typedef enum InjectionKind {
  INJECT_RECORD,
  INJECT_COMMANDS,
  INJECT_EID,
  INJECT_NMI,
  INJECT_KINDS,
} InjectionKind;

static volatile struct {
  uint64_t count;
  uint64_t cause;
  uint64_t epc;
  uint64_t mnstatus;
  uint64_t records;
  uint64_t status;
  uint64_t address;
} nmi;

static inline uint64_t mmio_read(uintptr_t addr) {
  return *(volatile uint64_t *)addr;
}

static inline void mmio_write(uintptr_t addr, uint64_t value) {
  *(volatile uint64_t *)addr = value;
  asm volatile("fence iorw, iorw" ::: "memory");
}

static inline uint32_t mmio_read32(uintptr_t addr) {
  return *(volatile uint32_t *)addr;
}

static inline void mmio_write32(uintptr_t addr, uint32_t value) {
  *(volatile uint32_t *)addr = value;
  asm volatile("fence iorw, iorw" ::: "memory");
}

static uint64_t reri_read(uint32_t record, uint64_t offset) {
  return mmio_read(RERI_RECORD(record) + offset);
}

static void reri_write(uint32_t record, uint64_t offset, uint64_t value) {
  mmio_write(RERI_RECORD(record) + offset, value);
}

static uint64_t test_address(uint32_t region, uint32_t record) {
  return 0x8000000000000000ULL + ((uint64_t)(region + record) << 16);
}

static int record_is_valid(uint32_t record) {
  return (reri_read(record, RERI_STATUS) & RERI_V) != 0;
}

static int wait_for_record_state(uint32_t record, int expected_valid) {
  for (uint32_t i = 0; i < MAX_WAIT; i++) {
    if (record_is_valid(record) == expected_valid) return 0;
    asm volatile("nop");
  }
  return -1;
}

static int wait_invalid(uint32_t record) {
  return wait_for_record_state(record, 0);
}

static int wait_valid(uint32_t record) {
  return wait_for_record_state(record, 1);
}

static int clear_record(uint32_t record) {
  reri_write(record, RERI_CTRL, RERI_ELSE | RERI_CECE | RERI_SRDP | RERI_SINV);
  if (wait_invalid(record) != 0) return -1;
  reri_write(record, RERI_CTRL, RERI_CECE);
  reri_write(record, RERI_STATUS, 0);
  return 0;
}

static int test_header(void) {
  uint64_t vendor = mmio_read(RERI_BASE);
  uint64_t bank_info = mmio_read(RERI_BASE + RERI_BANK_INFO);
  uint64_t summary = mmio_read(RERI_BASE + RERI_VALID_SUMMARY);
  int failed = 0;

  if ((vendor & 0xffffU) != RERI_EXPECTED_VENDOR ||
      ((bank_info >> 16) & 0x3fU) != RERI_RECORDS ||
      (bank_info >> 56) != RERI_EXPECTED_VERSION ||
      (summary & RERI_SUMMARY_SV) == 0) {
    printf("header mismatch vendor=0x%llx bank=0x%llx summary=0x%llx\n",
           (unsigned long long)vendor, (unsigned long long)bank_info,
           (unsigned long long)summary);
    failed = 1;
  }
  printf("header: %s\n", failed ? "FAIL" : "PASS");
  return failed;
}

static int test_record(uint32_t record) {
  const uint64_t address = test_address(TEST_ADDR_RECORD, record);
  const uint64_t payload = RERI_V | RERI_CE | RERI_CECO | (0xbeefULL << 48);
  int failed = 0;

  if (clear_record(record) != 0) return 1;
  reri_write(record, RERI_CTRL, RERI_ELSE | RERI_CECE);
  reri_write(record, RERI_ADDRESS, address);
  reri_write(record, RERI_STATUS, payload);

  if (reri_read(record, RERI_STATUS) != payload ||
      reri_read(record, RERI_ADDRESS) != address ||
      (mmio_read(RERI_BASE + RERI_VALID_SUMMARY) & (1ULL << (record + 1))) == 0) {
    printf("record %u visibility mismatch status=0x%llx address=0x%llx\n", record,
           (unsigned long long)reri_read(record, RERI_STATUS),
           (unsigned long long)reri_read(record, RERI_ADDRESS));
    failed = 1;
  }

  reri_write(record, RERI_STATUS, 0);
  if (reri_read(record, RERI_STATUS) != payload) {
    printf("record %u accepted a status write while valid\n", record);
    failed = 1;
  }

  return failed;
}

static int test_commands(uint32_t record) {
  uint64_t control = RERI_ELSE | RERI_CECE;
  uint64_t payload = RERI_V | RERI_CE | RERI_CECO | (0x1357ULL << 48);
  int failed = 0;

  if (clear_record(record) != 0) return 1;
  reri_write(record, RERI_CTRL, control);
  reri_write(record, RERI_STATUS, payload);
  reri_write(record, RERI_CTRL, control | RERI_SRDP);

  if (reri_read(record, RERI_STATUS) != (payload | RERI_RDIP)) {
    printf("record %u SRDP did not set RDIP\n", record);
    failed = 1;
  }
  reri_write(record, RERI_CTRL, control | RERI_SINV);
  if (wait_invalid(record) != 0 ||
      (reri_read(record, RERI_STATUS) & RERI_RDIP) == 0) {
    printf("record %u SINV did not clear V while preserving RDIP\n", record);
    failed = 1;
  }
  if ((reri_read(record, RERI_CTRL) & (RERI_ELSE | RERI_CECE)) != control) {
    printf("record %u command changed Control configuration\n", record);
    failed = 1;
  }

  return failed;
}

static int test_eid(uint32_t record) {
  const uint64_t address = test_address(TEST_ADDR_EID, record);
  const uint64_t expected = RERI_V | RERI_CE | RERI_RDIP;
  int failed = 0;

  if (clear_record(record) != 0) return 1;
  reri_write(record, RERI_ADDRESS, address);
  reri_write(record, RERI_STATUS, RERI_CE);
  reri_write(record, RERI_CTRL,
             RERI_ELSE | RERI_CECE | (1ULL << RERI_EID_SHIFT));
  if (wait_valid(record) != 0 ||
      (reri_read(record, RERI_STATUS) & expected) != expected ||
      reri_read(record, RERI_ADDRESS) != address) {
    printf("record %u EID completion mismatch status=0x%llx address=0x%llx\n", record,
           (unsigned long long)reri_read(record, RERI_STATUS),
           (unsigned long long)reri_read(record, RERI_ADDRESS));
    failed = 1;
  }

  return failed;
}

static int test_mmio32(uint32_t record) {
  const uintptr_t address_reg = RERI_RECORD(record) + RERI_ADDRESS;
  uint64_t expected = 0x1122334455667788ULL;
  int failed = 0;

  if (clear_record(record) != 0) return 1;
  reri_write(record, RERI_ADDRESS, expected);
  mmio_write32(address_reg, 0xaabbccddU);
  expected = 0x11223344aabbccddULL;
  if (mmio_read32(address_reg) != 0xaabbccddU ||
      mmio_read32(address_reg + 4) != 0x11223344U ||
      mmio_read(address_reg) != expected) {
    printf("record %u low-half MMIO32 write did not preserve the high half\n", record);
    failed = 1;
  }
  mmio_write32(address_reg + 4, 0x55667788U);
  expected = 0x55667788aabbccddULL;
  if (mmio_read(address_reg) != expected) {
    printf("record %u high-half MMIO32 mismatch got=0x%llx expected=0x%llx\n", record,
           (unsigned long long)mmio_read(address_reg),
           (unsigned long long)expected);
    failed = 1;
  }

  return failed;
}

static _Context *nmi_handler(_Event event, _Context *context) {
  nmi.cause = context->scause;
  nmi.epc = context->sepc;
  nmi.mnstatus = context->sstatus;
  nmi.records = 0;

  // Retire every enabled UEC source before mnret re-enables NMI delivery.
  for (uint32_t record = 0; record < RERI_RECORDS; record++) {
    uint64_t status = reri_read(record, RERI_STATUS);
    uint64_t control = reri_read(record, RERI_CTRL);
    if ((status & (RERI_V | RERI_UEC)) != (RERI_V | RERI_UEC) ||
        (control & RERI_UECS_MASK) == 0) continue;

    nmi.records |= 1ULL << record;
    if (record == RERI_SOFTWARE_RECORD) {
      nmi.status = status;
      nmi.address = reri_read(record, RERI_ADDRESS);
    }
    reri_write(record, RERI_CTRL, control | RERI_SRDP | RERI_SINV);
  }
  nmi.count++;
  return context;
}

static int test_nmi(uint32_t record) {
  const uint64_t address = test_address(TEST_ADDR_NMI, record);
  const uint64_t control = RERI_ELSE | RERI_CECE | (2ULL << RERI_UECS_SHIFT);
  const uint64_t expected_status = RERI_V | RERI_UEC | RERI_RDIP;
  const uint64_t before = nmi.count;
  int failed = 0;

  if (clear_record(record) != 0) return 1;
  reri_write(record, RERI_ADDRESS, address);
  reri_write(record, RERI_STATUS, RERI_UEC);
  reri_write(record, RERI_CTRL, control | (1ULL << RERI_EID_SHIFT));

  // V starts clear and is cleared by the handler, so it cannot signal completion.
  for (uint32_t i = 0; i < MAX_WAIT && nmi.count == before; i++) {
    asm volatile("nop");
  }
  if (nmi.count != before + 1 || nmi.cause != RERI_NMI_CAUSE ||
      nmi.epc == 0 || (nmi.mnstatus & MNSTATUS_NMIE) != 0 ||
      nmi.records != (1ULL << record) || nmi.status != expected_status ||
      nmi.address != address) {
    printf("record %u NMI mismatch count=%llu expected=%llu cause=0x%llx "
           "epc=0x%llx mnstatus=0x%llx records=0x%llx status=0x%llx address=0x%llx\n",
           record, (unsigned long long)nmi.count, (unsigned long long)(before + 1),
           (unsigned long long)nmi.cause, (unsigned long long)nmi.epc,
           (unsigned long long)nmi.mnstatus, (unsigned long long)nmi.records,
           (unsigned long long)nmi.status, (unsigned long long)nmi.address);
    failed = 1;
  }

  if (wait_invalid(record) != 0 ||
      reri_read(record, RERI_STATUS) != (expected_status & ~RERI_V) ||
      reri_read(record, RERI_CTRL) != control ||
      reri_read(record, RERI_ADDRESS) != address ||
      (mmio_read(RERI_BASE + RERI_VALID_SUMMARY) & (1ULL << (record + 1))) != 0) {
    printf("record %u NMI cleanup/return mismatch\n", record);
    failed = 1;
  }

  for (uint32_t i = 0; i < NMI_SETTLE_CYCLES; i++) asm volatile("nop");
  if (nmi.count != before + 1 || record_is_valid(record)) {
    printf("record %u NMI retriggered after cleanup\n", record);
    failed = 1;
  }
  if (clear_record(record) != 0) failed = 1;
  return failed;
}

int main() {
  int header_failed = 0;
  int record_failed = 0;
  int command_failed = 0;
  int eid_failed = 0;
  int mmio32_failed = 0;
  int nmi_failed = 0;

  if (_nmi_init(nmi_handler) != 0) return 1;

  // Hardware ECC and producer routing remain in l1dcache-ecc-it.
  header_failed += test_header();
  printf("mmio32 start\n");
  for (uint32_t record = 0; record < RERI_RECORDS; record++) {
    mmio32_failed += test_mmio32(record);
  }

  printf("injections start cases=%u\n", RERI_INJECTIONS);
  for (uint32_t injection = 0; injection < RERI_INJECTIONS; injection++) {
    const uint32_t record = (injection / INJECT_KINDS) % RERI_RECORDS;

    switch (injection % INJECT_KINDS) {
      case INJECT_RECORD: record_failed += test_record(record); break;
      case INJECT_COMMANDS: command_failed += test_commands(RERI_SOFTWARE_RECORD); break;
      case INJECT_EID: eid_failed += test_eid(RERI_SOFTWARE_RECORD); break;
      case INJECT_NMI: nmi_failed += test_nmi(RERI_SOFTWARE_RECORD); break;
    }
    if ((injection + 1U) % RERI_PROGRESS_STEP == 0U) {
      printf("injections progress=%u\n", injection + 1U);
    }
  }
  int cleanup_failed = 0;
  for (uint32_t record = 0; record < RERI_RECORDS; record++) {
    cleanup_failed += clear_record(record) != 0;
  }

  int failed = header_failed + record_failed + command_failed + eid_failed +
               mmio32_failed + nmi_failed + cleanup_failed;
  printf("record: %s\n", record_failed ? "FAIL" : "PASS");
  printf("commands: %s\n", command_failed ? "FAIL" : "PASS");
  printf("eid: %s\n", eid_failed ? "FAIL" : "PASS");
  printf("mmio32: %s\n", mmio32_failed ? "FAIL" : "PASS");
  printf("nmi: %s count=%llu\n", nmi_failed ? "FAIL" : "PASS",
         (unsigned long long)nmi.count);
  printf("cleanup: %s\n", cleanup_failed ? "FAIL" : "PASS");
  printf("reri_test injections=%u failed=%d\n", RERI_INJECTIONS, failed);
  return failed;
}
