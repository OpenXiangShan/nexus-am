include $(AM_HOME)/am/arch/isa/x86.mk
include $(AM_HOME)/am/arch/platform/nemu.mk

AM_SRCS += nemu/isa/x86/trm.c \
           nemu/isa/x86/cte.c \
           nemu/isa/x86/trap.S \
           nemu/isa/x86/vme.c \
           nemu/isa/x86/boot/start.S

CFLAGS  += -mstringop-strategy=loop -DISA_H=\"x86.h\"
LDFLAGS += -T $(AM_HOME)/am/src/nemu/isa/x86/boot/loader.ld
