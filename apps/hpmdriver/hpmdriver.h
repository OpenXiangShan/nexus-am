#ifndef __HPMDRIVER_H_
#define __HPMDRIVER_H_

#include <klib.h>
#include <csr.h>
#include "events.h"

#define MODE_OFFSET 59
#define MODE_MASK 0x1FUL
#define MODE_M 0x10UL
#define MODE_H 0x08UL
#define MODE_S 0x04UL
#define MODE_U 0x02UL
#define MODE_D 0x01UL

#define OPTYPE2_OFFSET 50
#define OPTYPE2_MASK 0x1FUL
#define OPTYPE1_OFFSET 45
#define OPTYPE1_MASK 0x1FUL
#define OPTYPE0_OFFSET 40
#define OPTYPE0_MASK 0x1FUL
#define OPTYPE_OR 0x0UL
#define OPTYPE_AND 0x1UL
#define OPTYPE_XOR 0x2UL
#define OPTYPE_ADD 0x4UL

#define EVENT3_OFFSET 30
#define EVENT3_MASK 0x3FFUL
#define EVENT2_OFFSET 20
#define EVENT2_MASK 0x3FFUL
#define EVENT1_OFFSET 10
#define EVENT1_MASK 0x3FFUL
#define EVENT0_OFFSET 0
#define EVENT0_MASK 0x3FFUL

#define SET(reg, field, value) ((reg) = ((reg) & ~((field##_MASK) << (field##_OFFSET))) | ((value) << (field##_OFFSET)));



#endif /* __HPMDRIVER_H_ */