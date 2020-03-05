include $(AM_HOME)/am/arch/isa/mips32.mk
include $(AM_HOME)/am/arch/platform/nemu.mk

AM_SRCS += nemu/isa/mips/trm.c \
           nemu/isa/mips/cte.c \
           nemu/isa/mips/trap.S \
           nemu/isa/mips/vme.c \
           nemu/isa/mips/boot/start.S

CFLAGS  += -DISA_H=\"mips.h\"
LDFLAGS += -T $(AM_HOME)/am/src/nemu/isa/mips/boot/loader.ld
