ARCH = mips32-npc

CROSS_COMPILE = mips-linux-gnu-
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

$(shell mkdir -p build/)

# AM library
AM_PATH = ./am/arch/$(ARCH)
AM_LIB = ./build/libam-$(ARCH).a
AM_SRC = $(shell find -L $(AM_PATH)/src -name "*.c" -o -name "*.cpp" -o -name "*.S")
AM_OBJ = $(addsuffix .o, $(basename $(AM_SRC)))

# TODO: managing flags

CFLAGS += -std=gnu99 -I ./am/ -I./$(AM_PATH)/include -O2 -MD -Wall -Werror -ggdb
CXXFLAGS += -std=c++11 -I ./am/ -I./$(AM_PATH)/include -O2 -MD -Wall -Werror -ggdb
ASFLAGS += -I ./am/ -I./$(AM_PATH)/include

# mips32-npc
CFLAGS += -fno-pic -static -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-delayed-branch -mno-abicalls -march=mips32 -D DEPLOY

# x86-qemu
# CFLAGS += -m32 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer
#CXXFLAGS += -m32 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer -ffreestanding -fno-rtti -fno-exceptions
# ASFLAGS += -m32

all: $(AM_LIB) build/a
	@true

$(AM_LIB): $(AM_OBJ)
	ar rcs $(AM_LIB) $(AM_OBJ)

# These are temporary.
# TODO: generic application making
build/a:
	$(CC) $(CFLAGS) -c -o build/hello.o apps/hello/src/hello.c -I./apps/hello/include
	$(CC) $(CFLAGS) -c -o build/print.o apps/hello/src/print.c -I./apps/hello/include
	ar rcs ./build/hello.a build/hello.o build/print.o
	$(AM_PATH)/img/build build/a.out $(shell readlink -f ./build/hello.a) $(shell readlink -f $(AM_LIB))
	@echo "====== execute ======"
	@qemu-system-i386 -serial stdio build/a.out

clean:
	rm -rf build/ $(shell find . -name "*.o" -o -name "*.d")
