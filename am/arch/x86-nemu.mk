CFLAGS  += -m32 -fno-pic -fno-omit-frame-pointer -march=i386 -mstringop-strategy=loop -fno-reorder-functions
LDFLAGS += -melf_i386
ASFLAGS += -m32 -fno-pic

include $(AM_HOME)/am/arch/nemu/nemu.mk
