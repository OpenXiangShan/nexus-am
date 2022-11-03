#include "hpmdriver.h"

int main() {
    printf("Hello, XiangShan!\n");
    printu_csr(marchid);
    printx_csr(mcountinhibit);
    printx_csr(mcounteren);
    printx_csr(scounteren);

    printu_csr(mcycle);
    printu_csr(minstret);

    se_cc_single(3, MODE_M, Frontend_frontendFlush);
    se_cc_single(11, MODE_M, CtrlBlock_decoder_waitInstr);
    se_cc_double(19, MODE_M, OPTYPE_ADD, MemBlock_loadpipe0_load_req, MemBlock_loadpipe1_load_req);

    // === tmp workload ===
    volatile uint64_t a = 0;
    for(uint64_t i = 0; i < 100; i++) {
        a += a + i;
    }
    printf("%lu\n",a);

    print_event(3);
    print_counter(3);
    print_event(11);
    print_counter(11);
    print_event(19);
    print_counter(19);

    return 0;
}