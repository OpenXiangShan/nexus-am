CROSS_COMPILE = mips-linux-gnu-
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -MD -fno-pic -static -fno-strict-aliasing -fno-builtin -EB
CFLAGS += -fno-stack-protector -mno-check-zero-division -fno-delayed-branch
CFLAGS += -Wall -Werror -march=mips32 -mno-llsc
CFLAGS += -mno-mad -std=gnu99
CFLAGS += -I include -O2 -Wno-unused-function
CFLAGS += -mno-abicalls -g
CFLAGS += -D DEPLOY
SCFLAGS = -MD -fno-pic -static -fno-strict-aliasing -fno-builtin
SCFLAGS += -fno-stack-protector -mno-check-zero-division -fno-delayed-branch
SCFLAGS += -Wall -Werror -march=mips32 -mno-llsc -mno-imadd -mno-mad
SCFLAGS += -I ../include -O2 -S -Wno-unused-function
ASFLAGS = -MD -mno-check-zero-division -mips32 -O0 -mno-abicalls
ASFLAGS += -fno-pic -fno-delayed-branch
LDFLAGS = 

OBJECTS = $(shell find -L . -name "*.o")

mips-arch:
	@cd boot; make;
	@cd kernel; make;
	@cd ../../build; rm -rf mips; mkdir mips;

OBJ:
	-cp $(OBJECTS) ../../build/mips/

clean:
	@cd boot; make clean;
	@cd kernel; make clean;
	rm -rf ../../build/mips
