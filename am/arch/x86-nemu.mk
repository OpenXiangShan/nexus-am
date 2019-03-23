include $(AM_HOME)/am/arch/isa/x86.mk
CFLAGS += -mstringop-strategy=loop -fno-reorder-functions
include $(AM_HOME)/am/arch/platform/nemu.mk
