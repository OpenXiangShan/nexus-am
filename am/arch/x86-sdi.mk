include $(AM_HOME)/am/arch/isa/x86.mk
include $(AM_HOME)/am/arch/platform/sdi.mk
CFLAGS  += -mstringop-strategy=loop
AM_SRCS += nemu/isa/x86/boot/start.S
