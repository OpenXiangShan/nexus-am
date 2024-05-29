#ifndef __COUNTER_H__
#define __COUNTER_H__

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

#define CSRS_UNPRIV_COUNTER_TIMERS(f) \
    CSRS_UNPRIV_CNTR(f) \
    CSRS_UNPRIV_HPM(f)

#define CSRS_M_CNTR(f) \
    f(mcycle     , 0xB00) f(minstret   , 0xB02)

#define CSRS_M_HPM(f) \
    f(mhpmcounter3   , 0xB03) \
    f(mhpmcounter4   , 0xB04) f(mhpmcounter5   , 0xB05) f(mhpmcounter6   , 0xB06) f(mhpmcounter7   , 0xB07) \
    f(mhpmcounter8   , 0xB08) f(mhpmcounter9   , 0xB09) f(mhpmcounter10  , 0xB0A) f(mhpmcounter11  , 0xB0B) \
    f(mhpmcounter12  , 0xB0C) f(mhpmcounter13  , 0xB0D) f(mhpmcounter14  , 0xB0E) f(mhpmcounter15  , 0xB0F) \
    f(mhpmcounter16  , 0xB10) f(mhpmcounter17  , 0xB11) f(mhpmcounter18  , 0xB12) f(mhpmcounter19  , 0xB13) \
    f(mhpmcounter20  , 0xB14) f(mhpmcounter21  , 0xB15) f(mhpmcounter22  , 0xB16) f(mhpmcounter23  , 0xB17) \
    f(mhpmcounter24  , 0xB18) f(mhpmcounter25  , 0xB19) f(mhpmcounter26  , 0xB1A) f(mhpmcounter27  , 0xB1B) \
    f(mhpmcounter28  , 0xB1C) f(mhpmcounter29  , 0xB1D) f(mhpmcounter30  , 0xB1E) f(mhpmcounter31  , 0xB1F)

#define CSRS_M_COUNTER_TIMERS(f)\
    CSRS_M_CNTR(f) \
    CSRS_M_HPM(f)

typedef uint64_t (*func_csr_read_t)();
typedef void (*func_csr_write_t)(uint64_t);

extern func_csr_read_t arr_func_read_machine_counter[COUNTER_NUM];
extern func_csr_write_t arr_func_write_machine_counter[COUNTER_NUM];
extern func_csr_read_t arr_func_read_unpriv_counter[COUNTER_NUM];

#define def_func_csr_read(csr, addr) \
    uint64_t csr_read_##csr() { \
        return csr_read(csr); \
    }

#define list_func_csr_read(csr, addr) \
    csr_read_##csr,

#define def_func_csr_write(csr, addr) \
    void csr_write_##csr(uint64_t src) { \
        csr_write(csr, src); \
    }

#define list_func_csr_write(csr, addr) \
    csr_write_##csr,


#endif // __COUNTER_H__
