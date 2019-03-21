CROSS_COMPILE := riscv-none-embed-
LDFLAGS       += -melf32lriscv
CFLAGS        += -fno-pic -static -march=rv32im -mabi=ilp32
ASFLAGS       += -march=rv32im -mabi=ilp32 -O0 -fno-pic

include $(AM_HOME)/am/arch/nemu/nemu.mk
