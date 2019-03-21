LDFLAGS       := -EL
CROSS_COMPILE := mips-linux-gnu-
CFLAGS        += -fno-pic -static -fno-delayed-branch -mno-abicalls -march=mips32 -mno-check-zero-division -EL -mno-llsc -mno-imadd -mno-mad
ASFLAGS       += -march=mips32 -EL -mno-check-zero-division -O0 -mno-abicalls -fno-pic -fno-delayed-branch

include $(AM_HOME)/am/arch/nemu/nemu.mk
