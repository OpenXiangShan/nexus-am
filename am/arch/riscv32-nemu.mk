include $(AM_HOME)/am/arch/isa/riscv32.mk
include $(AM_HOME)/am/arch/platform/nemu.mk

AM_SRCS += nemu/isa/riscv/trm.c \
           nemu/isa/riscv/cte.c \
           nemu/isa/riscv/trap.S \
           nemu/isa/riscv/vme.c \
           nemu/isa/riscv/boot/start.S

CFLAGS  += -DISA_H=\"riscv.h\"
LDFLAGS += -T $(AM_HOME)/am/src/nemu/isa/riscv/boot/loader.ld
