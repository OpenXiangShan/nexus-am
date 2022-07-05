#include "hpmdriver.h"

#define PRINT_SEP(...) printf("-------------------- %s --------------------\n", __VA_ARGS__);

int main() {
    printf("Hello, XiangShan!\n");
    printf("marchid: %ld\n", csr_read(marchid));
    printf("mcountinhibit: %ld\n", csr_read(mcountinhibit));
    printf("mcounteren: %ld\n", csr_read(mcounteren));
    printf("scounteren: %ld\n", csr_read(scounteren));
    
    printf("mcycle: %ld\n", csr_read(mcycle));
    printf("minstret: %ld\n", csr_read(minstret));

    se_cc_single(3, MODE_M, Frontend_frontendFlush);
    se_cc_single(11, MODE_M, Frontend_frontendFlush);

    // === tmp workload ===
    volatile uint64_t a = 0;
    for(uint64_t i = 0; i < 100; i++) {
        a += a + i;
    }
    printf("%lu\n",a);
    // *** tmp workload ***

    print_event(3);
    print_counter(3);
    print_event(11);
    print_counter(11);

#if 0
    uint64_t set_mode_M = (0x1UL << 63);
    // ===== frontend ==================================================
    csr_write(mhpmevent3, csr_read(mhpmevent3)|set_mode_M);
    csr_write(mhpmevent4, csr_read(mhpmevent4)|set_mode_M|0x1UL);
    csr_write(mhpmevent5, csr_read(mhpmevent5)|set_mode_M|0x2UL);
    csr_write(mhpmevent6, csr_read(mhpmevent6)|set_mode_M|0x3UL);
    csr_write(mhpmevent7, csr_read(mhpmevent7)|set_mode_M|0x10UL);
    csr_write(mhpmevent8, csr_read(mhpmevent8)|set_mode_M|0xFFFFFFFFUL);
    csr_write(mhpmevent9, csr_read(mhpmevent9)|set_mode_M|0x00000081UL);
    csr_write(mhpmevent10, csr_read(mhpmevent10)|set_mode_M);
    // ======= ctrl ==================================================
    csr_write(mhpmevent11, csr_read(mhpmevent11)|set_mode_M|0x1UL);
    csr_write(mhpmevent12, csr_read(mhpmevent12)|set_mode_M|0x2UL);
    csr_write(mhpmevent13, csr_read(mhpmevent13)|set_mode_M|0x3UL);
    csr_write(mhpmevent14, csr_read(mhpmevent14)|set_mode_M|0x10UL);
    csr_write(mhpmevent15, csr_read(mhpmevent15)|set_mode_M|0xFFFFFFFFUL);
    csr_write(mhpmevent16, csr_read(mhpmevent16)|set_mode_M);
    csr_write(mhpmevent17, csr_read(mhpmevent17)|set_mode_M);
    csr_write(mhpmevent18, csr_read(mhpmevent18)|set_mode_M);
    // ======== memory ===================================================
    csr_write(mhpmevent19, csr_read(mhpmevent19)|set_mode_M);
    csr_write(mhpmevent20, csr_read(mhpmevent20)|set_mode_M|0x1UL);
    csr_write(mhpmevent21, csr_read(mhpmevent21)|set_mode_M|0x2UL);
    csr_write(mhpmevent22, csr_read(mhpmevent22)|set_mode_M|0x3UL);
    csr_write(mhpmevent23, csr_read(mhpmevent23)|set_mode_M|43);
    csr_write(mhpmevent24, csr_read(mhpmevent24)|set_mode_M|52);
    csr_write(mhpmevent25, csr_read(mhpmevent25)|set_mode_M|52);
    csr_write(mhpmevent26, csr_read(mhpmevent26)|set_mode_M|43);
    // ======== hc ================================================
    csr_write(mhpmevent27, csr_read(mhpmevent27)|set_mode_M);
    csr_write(mhpmevent28, csr_read(mhpmevent28)|set_mode_M|0x1UL);
    csr_write(mhpmevent29, csr_read(mhpmevent29)|set_mode_M|0x2UL);
    csr_write(mhpmevent30, csr_read(mhpmevent30)|set_mode_M|0x3UL);
    csr_write(mhpmevent31, csr_read(mhpmevent31)|set_mode_M|0xFFFFFFFFUL);

    // csr write mhpmcounter 3 to 31 0x0
    csr_write(mhpmcounter3, 0x0UL);
    csr_write(mhpmcounter4, 0x0UL);
    csr_write(mhpmcounter5, 0x0UL);
    csr_write(mhpmcounter6, 0x0UL);
    csr_write(mhpmcounter7, 0x0UL);
    csr_write(mhpmcounter8, 0x0UL);
    csr_write(mhpmcounter9, 0x0UL);
    csr_write(mhpmcounter10, 0x0UL);
    csr_write(mhpmcounter11, 0x0UL);
    csr_write(mhpmcounter12, 0x0UL);
    csr_write(mhpmcounter13, 0x0UL);
    csr_write(mhpmcounter14, 0x0UL);
    csr_write(mhpmcounter15, 0x0UL);
    csr_write(mhpmcounter16, 0x0UL);
    csr_write(mhpmcounter17, 0x0UL);
    csr_write(mhpmcounter18, 0x0UL);
    csr_write(mhpmcounter19, 0x0UL);
    csr_write(mhpmcounter20, 0x0UL);
    csr_write(mhpmcounter21, 0x0UL);
    csr_write(mhpmcounter22, 0x0UL);
    csr_write(mhpmcounter23, 0x0UL);
    csr_write(mhpmcounter24, 0x0UL);
    csr_write(mhpmcounter25, 0x0UL);
    csr_write(mhpmcounter26, 0x0UL);
    csr_write(mhpmcounter27, 0x0UL);
    csr_write(mhpmcounter28, 0x0UL);
    csr_write(mhpmcounter29, 0x0UL);
    csr_write(mhpmcounter30, 0x0UL);
    csr_write(mhpmcounter31, 0x0UL);
    csr_write(mcycle, 0x0UL);
    csr_write(minstret, 0x0UL);

    // ================================================================
    volatile uint64_t a = 0;
    for(uint64_t i = 0; i < 100; i++) {
        a += a + i;
    }
    printf("%lu\n",a);
    // ================================================================
    PRINT_SEP("frontend");
    printf("mhpmevent3: %lx, mhpmcounter3: %ld\n", csr_read(mhpmevent3), csr_read(mhpmcounter3));
    printf("mhpmevent4: %lx, mhpmcounter4: %ld\n", csr_read(mhpmevent4), csr_read(mhpmcounter4));
    printf("mhpmevent5: %lx, mhpmcounter5: %ld\n", csr_read(mhpmevent5), csr_read(mhpmcounter5));
    printf("mhpmevent6: %lx, mhpmcounter6: %ld\n", csr_read(mhpmevent6), csr_read(mhpmcounter6));
    printf("mhpmevent7: %lx, mhpmcounter7: %ld\n", csr_read(mhpmevent7), csr_read(mhpmcounter7));
    printf("mhpmevent8: %lx, mhpmcounter8: %ld\n", csr_read(mhpmevent8), csr_read(mhpmcounter8));
    printf("mhpmevent9: %lx, mhpmcounter9: %ld\n", csr_read(mhpmevent9), csr_read(mhpmcounter9));
    printf("mhpmevent10: %lx, mhpmcounter10: %ld\n", csr_read(mhpmevent10), csr_read(mhpmcounter10));
    PRINT_SEP("ctrl");    
    printf("mhpmevent11: %lx, mhpmcounter11: %ld\n", csr_read(mhpmevent11), csr_read(mhpmcounter11));
    printf("mhpmevent12: %lx, mhpmcounter12: %ld\n", csr_read(mhpmevent12), csr_read(mhpmcounter12));
    printf("mhpmevent13: %lx, mhpmcounter13: %ld\n", csr_read(mhpmevent13), csr_read(mhpmcounter13));
    printf("mhpmevent14: %lx, mhpmcounter14: %ld\n", csr_read(mhpmevent14), csr_read(mhpmcounter14));
    printf("mhpmevent15: %lx, mhpmcounter15: %ld\n", csr_read(mhpmevent15), csr_read(mhpmcounter15));
    printf("mhpmevent16: %lx, mhpmcounter16: %ld\n", csr_read(mhpmevent16), csr_read(mhpmcounter16));
    printf("mhpmevent17: %lx, mhpmcounter17: %ld\n", csr_read(mhpmevent17), csr_read(mhpmcounter17));
    PRINT_SEP("memory");
    printf("mhpmevent18: %lx, mhpmcounter18: %ld\n", csr_read(mhpmevent18), csr_read(mhpmcounter18));
    printf("mhpmevent19: %lx, mhpmcounter19: %ld\n", csr_read(mhpmevent19), csr_read(mhpmcounter19));
    printf("mhpmevent20: %lx, mhpmcounter20: %ld\n", csr_read(mhpmevent20), csr_read(mhpmcounter20));
    printf("mhpmevent21: %lx, mhpmcounter21: %ld\n", csr_read(mhpmevent21), csr_read(mhpmcounter21));
    printf("mhpmevent22: %lx, mhpmcounter22: %ld\n", csr_read(mhpmevent22), csr_read(mhpmcounter22));
    printf("mhpmevent23: %lx, mhpmcounter23: %ld\n", csr_read(mhpmevent23), csr_read(mhpmcounter23));
    printf("mhpmevent24: %lx, mhpmcounter24: %ld\n", csr_read(mhpmevent24), csr_read(mhpmcounter24));
    printf("mhpmevent25: %lx, mhpmcounter25: %ld\n", csr_read(mhpmevent25), csr_read(mhpmcounter25));
    printf("mhpmevent26: %lx, mhpmcounter26: %ld\n", csr_read(mhpmevent26), csr_read(mhpmcounter26));
    PRINT_SEP("hc");
    printf("mhpmevent27: %lx, mhpmcounter27: %ld\n", csr_read(mhpmevent27), csr_read(mhpmcounter27));
    printf("mhpmevent28: %lx, mhpmcounter28: %ld\n", csr_read(mhpmevent28), csr_read(mhpmcounter28));
    printf("mhpmevent29: %lx, mhpmcounter29: %ld\n", csr_read(mhpmevent29), csr_read(mhpmcounter29));
    printf("mhpmevent30: %lx, mhpmcounter30: %ld\n", csr_read(mhpmevent30), csr_read(mhpmcounter30));
    printf("mhpmevent31: %lx, mhpmcounter31: %ld\n", csr_read(mhpmevent31), csr_read(mhpmcounter31));

    printf("mcycle: %ld\n", csr_read(mcycle));
    printf("mhpmcounter8: %ld\n", csr_read(mhpmcounter8));
    printf("mhpmcounter15: %ld\n", csr_read(mhpmcounter15));
    printf("mhpmcounter24: %ld\n", csr_read(mhpmcounter24));

// ================================================================
    PRINT_SEP("end");
#endif

    return 0;
}