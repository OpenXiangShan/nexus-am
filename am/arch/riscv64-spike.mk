include $(AM_HOME)/am/arch/isa/riscv64.mk
include $(AM_HOME)/am/arch/platform/spike.mk

AM_SRCS += spike/isa/riscv/trm.c \
           nemu/isa/riscv/cte.c \
           nemu/isa/riscv/trap.S \
           nemu/isa/riscv/cte64.c \
           nemu/isa/riscv/mtime.S \
           nemu/isa/riscv/vme.c \
           spike/isa/riscv/boot/start.S

CFLAGS  += -DISA_H=\"riscv.h\"
LDFLAGS += -T $(AM_HOME)/am/src/nemu/isa/riscv/boot/loader64.ld
