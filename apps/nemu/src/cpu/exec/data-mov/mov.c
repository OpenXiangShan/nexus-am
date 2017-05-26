#include "cpu/exec/helper.h"
#include "x86-inc/mmu.h"
#include "x86-inc/addr_struct.h"

#define DATA_BYTE 1
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "mov-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(mov_i2r)
make_helper_v(mov_i2rm)
make_helper_v(mov_r2rm)
make_helper_v(mov_rm2r)
make_helper_v(mov_a2moffs)
make_helper_v(mov_moffs2a)
make_helper_v(mov_cr2r)
make_helper_v(mov_r2cr)
make_helper_v(mov_a2sreg)

