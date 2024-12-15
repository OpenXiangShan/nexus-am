#include <klib.h>
#include "./eccctrl.h"

#define TEST_LOG_LEVEL_ERROR 2
#define TEST_LOG_LEVEL_INFO 1
#define TEST_LOG_LEVEL_DEBUG 0
#define TEST_LOG_LEVEL TEST_LOG_LEVEL_DEBUG

#if TEST_LOG_LEVEL <= TEST_LOG_LEVEL_DEBUG
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#if TEST_LOG_LEVEL <= TEST_LOG_LEVEL_INFO
#define INFO(...) printf(__VA_ARGS__)
#else
#define INFO(...)
#endif

#if TEST_LOG_LEVEL <= TEST_LOG_LEVEL_ERROR
#define ERROR(...) printf(__VA_ARGS__)
#else
#define ERROR(...)
#endif

#define MAX_WAIT 100

int __attribute__((noinline)) target() {
    static int n = 0;
    printf("Hello, World @ %d!\n", n++);
    return n;
}

volatile uint64_t* const eccctrl = (uint64_t *)(ICACHECTRL_ECCCTRL_ADDR);
volatile uint64_t* const ecciaddr = (uint64_t *)(ICACHECTRL_ECCIADDR_ADDR);

int test(uint64_t ctrl, istatus_t expected_istatus, ierror_t expected_ierror, const char *desp) {
    static int test_id = 0;
    INFO("=== [%d] %s ===\n", test_id, desp);
    // set the control register and wait for the operation to finish
    DEBUG("ECC inject start! eccctrl=0x%x\n", ctrl);
    *eccctrl = ctrl;
    istatus_t istatus;
    int wait = 0;
    do {
        ctrl = *eccctrl;
        istatus = ISTATUS_GET(ctrl);

        if (wait++ > MAX_WAIT) {
            ERROR("Timeout! eccctrl=0x%x\n", ctrl);
            goto fail;
        }

    } while (istatus == ISTATUS_WORKING);

    DEBUG("ECC inject done! eccctrl=0x%x\n", ctrl);

    if (istatus != expected_istatus) {
        ERROR("Unexpected istatus: %d, should be: %d\n", istatus, expected_istatus);
        goto fail;
    }

    if (istatus == ISTATUS_ERROR) {
        ierror_t ierror = IERROR_GET(ctrl);
        if (ierror != expected_ierror) {
            ERROR("Unexpected ierror: %d, should be: %d\n", ierror, expected_ierror);
            goto fail;
        }
    } else if (istatus == ISTATUS_INJECTED) {
        DEBUG("No error, call target()\n"); // trigger the icache to read the data & auto-correct
        target();
    }

    INFO("=== [%d] Test passed! ===\n\n", test_id++);
    return 0;

fail:
    INFO("=== [%d] Test failed! ===\n\n", test_id++);
    return 1;
}

int main() {
    INFO("Setup\n");
    target(); // call target() first to make sure it is in the icache
    *ecciaddr = (uint64_t) target;

    INFO("Test\n");
    int failed = 0;

    failed += test(ITARGET_SET(ITARGET_META) | INJECT | ENABLE, ISTATUS_INJECTED, 0, "Inject metaArray");
    failed += test(ITARGET_SET(ITARGET_DATA) | INJECT | ENABLE, ISTATUS_INJECTED, 0, "Inject dataArray");
    failed += test(ITARGET_SET(ITARGET_RSVD1) | INJECT | ENABLE, ISTATUS_ERROR, IERROR_TARGET_INVALID, "Inject to invalid target");
    failed += test(ITARGET_SET(ITARGET_META) | INJECT, ISTATUS_ERROR, IERROR_NOT_ENABLED, "Inject when ecc not enabled");

    *ecciaddr = 0; // set to where definitely cannot be in the icache
    failed += test(ITARGET_SET(ITARGET_META) | INJECT | ENABLE, ISTATUS_ERROR, IERROR_NOT_FOUND, "Inject to invalid address");

    return failed;
}
