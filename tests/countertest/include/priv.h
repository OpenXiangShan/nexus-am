#ifndef __PRIV_H__
#define __PRIV_H__

#include <stdint.h>

#define ECALL_GOTO_PRIV_MODE 1


enum {
    MODE_M = 0b11,
    MODE_S = 0b01,
    MODE_U = 0b00,
    
    MODE_HS = 0b001,
    MODE_VS = 0b101,
    MODE_VU = 0b100,
};

typedef struct
{
    uint64_t expected_trap;
    uint64_t actual_trap;
    uint64_t cause;
    uint64_t epc;
} exception_t;

extern exception_t last_exception;

typedef union {
    struct {
        uint64_t uie : 1;
        uint64_t sie : 1;
        uint64_t pad0: 1;
        uint64_t mie : 1;
        uint64_t upie: 1;
        uint64_t spie: 1;
        uint64_t pad1: 1;
        uint64_t mpie: 1;
        uint64_t spp : 1;
        uint64_t vs  : 2;
        uint64_t mpp : 2;
        uint64_t fs  : 2;
        uint64_t xs  : 2;
        uint64_t mprv: 1;
        uint64_t sum : 1;
        uint64_t mxr : 1;
        uint64_t tvm : 1;
        uint64_t tw  : 1;
        uint64_t tsr : 1;
        uint64_t pad3: 9;
        uint64_t uxl : 2;
        uint64_t sxl : 2;
        uint64_t sbe : 1;
        uint64_t mbe : 1;
        uint64_t gva : 1;
        uint64_t mpv : 1;
        uint64_t pad4:23;
        uint64_t sd  : 1;
    }; 
    uint64_t val;
} mstatus_t;

volatile int m_exception_handler(
    uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
    uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7
);
volatile int ecall_handler(
    uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
    uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t ecall_num
);
int ecall_goto_priv_mode_handler(uint64_t next_mode);
void goto_priv_mode(uint64_t next_mode);

void do_ecall(
    uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
    uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t ecall_num
);

void setup_expected_exception();
void clear_last_exception();

#endif // __PRIV_H__
