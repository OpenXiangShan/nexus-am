#include "cpu/exec/helper.h"

#define DATA_BYTE 2
#include "sbb-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "sbb-template.h"
#undef DATA_BYTE

make_helper_v(sbb_rm2r)
make_helper_v(sbb_r2rm)

