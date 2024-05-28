#ifndef __COUNTERTEST_H__
#define __COUNTERTEST_H__

#define MSG_ERROR "\33[1;31mERROR\033[0m"

#define MAP(c, f) c(f)

#define MACHINE_COUNTER_BASE    0xB00
#define UNPRIV_COUNTER_BASE     0xC00
#define COUNTER_NUM             32

#define CSRS_UNPRIV_CNTR(f) \
    f(cycle      , 0xC00) \
    f(time       , 0xC01) \
    f(instret    , 0xC02)

#define CSRS_UNPRIV_HPM(f) \
    f(hpmcounter3    , 0xC03) \
    f(hpmcounter4    , 0xC04) f(hpmcounter5    , 0xC05) f(hpmcounter6    , 0xC06) f(hpmcounter7    , 0xC07) \
    f(hpmcounter8    , 0xC08) f(hpmcounter9    , 0xC09) f(hpmcounter10   , 0xC0A) f(hpmcounter11   , 0xC0B) \
    f(hpmcounter12   , 0xC0C) f(hpmcounter13   , 0xC0D) f(hpmcounter14   , 0xC0E) f(hpmcounter15   , 0xC0F) \
    f(hpmcounter16   , 0xC10) f(hpmcounter17   , 0xC11) f(hpmcounter18   , 0xC12) f(hpmcounter19   , 0xC13) \
    f(hpmcounter20   , 0xC14) f(hpmcounter21   , 0xC15) f(hpmcounter22   , 0xC16) f(hpmcounter23   , 0xC17) \
    f(hpmcounter24   , 0xC18) f(hpmcounter25   , 0xC19) f(hpmcounter26   , 0xC1A) f(hpmcounter27   , 0xC1B) \
    f(hpmcounter28   , 0xC1C) f(hpmcounter29   , 0xC1D) f(hpmcounter30   , 0xC1E) f(hpmcounter31   , 0xC1F)

#define CSR_ALL_UNPRIV_COUNTER(f) \
    CSRS_UNPRIV_CNTR(f) \
    CSRS_UNPRIV_HPM(f)

#define ACCESSIBLE 0
#define EX_II 2
#define EX_VI 22

extern int error;


#endif // __COUNTERTEST_H__
