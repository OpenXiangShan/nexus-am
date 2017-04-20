ifneq ($(MAKECMDGOALS), clean)
ifeq ($(ARCH), )
$(error "Usage: make [play|clean] ARCH=[mips32-npc|x86-linux|x86-qemu] APP=[hello,video,...]")
endif
endif

ifeq ($(ARCH), mips32-npc)
CROSS_COMPILE = mips-linux-gnu-
endif
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld

$(shell mkdir -p build/)

# AM library archive
AM_PATH = ./am/arch/$(ARCH)
AM_LIB  = ./build/libam-$(ARCH).a
AM_SRC  = $(shell find -L $(AM_PATH)/src -name "*.c" -o -name "*.cpp" -o -name "*.S")
AM_OBJ  = $(addsuffix .o, $(basename $(AM_SRC)))

# Application archive
APP_PATH = ./apps/$(APP)
APP_LIB  = ./build/$(APP)-$(ARCH).a
APP_SRC  = $(shell find -L $(APP_PATH)/src -name "*.c" -o -name "*.cpp" -o -name "*.S")
APP_OBJ  = $(addsuffix .o, $(basename $(APP_SRC)))

# Basic compilation flags
CFLAGS   += -std=gnu99 -I ./am/ -I./$(AM_PATH)/include -I./$(APP_PATH)/include -O2 -MD -Wall -Werror -ggdb
CXXFLAGS += -std=c++11 -I ./am/ -I./$(AM_PATH)/include -I./$(APP_PATH)/include -O2 -MD -Wall -Werror -ggdb
ASFLAGS  +=            -I ./am/ -I./$(AM_PATH)/include -I./$(APP_PATH)/include

# Arch-dependent compilation flags
ifeq ($(ARCH), mips32-npc)
CXX = $(CROSS_COMPILE)g++-5
CFLAGS   += -fno-pic -static -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-delayed-branch -mno-abicalls -march=mips32
CXXFLAGS += -fno-pic -static -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-delayed-branch -mno-abicalls -march=mips32 -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -march=mips32
endif
ifeq ($(ARCH), x86-qemu)
CFLAGS   += -m32 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer
CXXFLAGS += -m32 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -m32
endif

# The final binary
DEST = build/$(APP)-$(ARCH)

$(DEST): $(AM_LIB) $(APP_LIB)
	$(AM_PATH)/img/build $(DEST) $(shell readlink -f $(APP_LIB)) $(shell readlink -f $(AM_LIB))

# AM library
$(AM_LIB): $(AM_OBJ)
	ar rcs $(AM_LIB) $(AM_OBJ)

# Application
$(APP_LIB): $(APP_OBJ)
	ar rcs $(APP_LIB) $(APP_OBJ)

.PHONY: play clean

play: $(AM_LIB) $(DEST)
ifeq ($(ARCH), mips32-npc)
	@echo "Burn it to FPGA."
endif
ifeq ($(ARCH), x86-linux)
	@$(DEST)
endif
ifeq ($(ARCH), x86-qemu)
	@qemu-system-i386 -serial stdio $(DEST)
endif

clean:
	rm -rf build/ $(shell find . -name "*.o" -o -name "*.d")

