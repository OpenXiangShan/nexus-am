CROSS_COMPILE = mips-linux-gnu-
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -MD -fno-pic -static -fno-strict-aliasing -fno-builtin -EB
CFLAGS += -fno-stack-protector -mno-check-zero-division -fno-delayed-branch
CFLAGS += -Wall -Werror -march=mips32 -mno-llsc
# CFLAGS += -mno-imadd
CFLAGS += -mno-mad -std=gnu99
CFLAGS += -I inc -I include -O2 -Wno-unused-function
CFLAGS += -mno-abicalls -g
CFLAGS += -D DEPLOY
SCFLAGS = -MD -fno-pic -static -fno-strict-aliasing -fno-builtin
SCFLAGS += -fno-stack-protector -mno-check-zero-division -fno-delayed-branch
SCFLAGS += -Wall -Werror -march=mips32 -mno-llsc -mno-imadd -mno-mad
SCFLAGS += -I../include -O2 -S -Wno-unused-function
ASFLAGS = -MD -mno-check-zero-division -mips32 -O0 -mno-abicalls
ASFLAGS += -fno-pic -fno-delayed-branch
LDFLAGS = 

CSRC = $(shell find -L . -name "*.c")
SSRC = $(shell find -L . -name "*.S")
OBJECTS = $(shell find -L . -name "*.o")

export CROSS_COMPILE

.PHONY: asm

# ********************
# Rules of Compilation
# ********************

OBJECTS: $(CSRC) $(SSRC)
	$(CC) $(CFLAGS) -c $(CSRC) $(SSRC)

cp:
	mkdir ../../../build/mips/boot
	cp $(OBJECTS) ../../../build/mips/boot

clean:
	-rm -f *.o *.d > /dev/null
