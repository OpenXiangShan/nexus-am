import sys

arch = "x86-qemu"

if "mips" in ARCH:
  cross_compile = "mips-linux-gnu-"
else:
  cross_compile = ""

exit()

tools = {
  'as': cross_compile + 'gcc',
  'cc': cross_compile + 'gcc',
  'cxx': cross_compile + 'g++',
  'ld': cross_compile + 'ld',
}

cflags = "-std=gnu99 -I./am/ -I./$(AM_PATH)/include -I./klib -I./$(APP_PATH)/include -O2 -MD -Wall -Werror -ggdb"
cxxflags = "-std=c++11 -I./am/ -I./$(AM_PATH)/include -I./klib -I./$(APP_PATH)/include -O2 -MD -Wall -Werror -ggdb"
asflags = "-I./am/ -I./$(AM_PATH)/include -I./$(APP_PATH)/include -MD"


'''
# Basic compilation flags
CFLAGS   += -std=gnu99 -I./am/ -I./$(AM_PATH)/include -I./klib -I./$(APP_PATH)/include -O2 -MD -Wall -Werror -ggdb
CXXFLAGS += ASFLAGS  +=            -I./am/ -I./$(AM_PATH)/include -I./$(APP_PATH)/include -MD

# Arch-dependent compilation flags
ifeq ($(ARCH), mips32-npc)
CXX = $(CROSS_COMPILE)g++-5
CFLAGS   += -MD -fno-pic -static -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-delayed-branch -mno-abicalls -march=mips32 -mno-check-zero-division -EL
CXXFLAGS += -fno-pic -static -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-delayed-branch -mno-abicalls -march=mips32 -ffreestanding -fno-rtti -fno-exceptions -mno-check-zero-division -EL
ASFLAGS  += -march=mips32 -EL -MD -mno-check-zero-division -O0 -mno-abicalls -fno-pic -fno-delayed-branch
endif
ifeq ($(ARCH), mips32-minimal)
CXX = $(CROSS_COMPILE)g++-5
CFLAGS   += -MD -fno-pic -static -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-delayed-branch -mno-abicalls -march=mips32 -mno-check-zero-division -EL
CXXFLAGS += -fno-pic -static -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-delayed-branch -mno-abicalls -march=mips32 -ffreestanding -fno-rtti -fno-exceptions -mno-check-zero-division -EL
ASFLAGS  += -march=mips32 -EL -MD -mno-check-zero-division -O0 -mno-abicalls -fno-pic -fno-delayed-branch
endif

ifeq ($(ARCH), x86-qemu)
CFLAGS   += -m32 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer
CXXFLAGS += -m32 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -m32
endif

# Compilation patterns
build/$(ARCH)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/$(ARCH)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

build/$(ARCH)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c -o $@ $<

# -----------------------------------------------------------------------------

# The final binary
DEST = build/$(APP)-$(ARCH)

.PHONY: image
image: $(AM_LIB) $(KLIB) $(APP_LIB)
	$(AM_PATH)/img/build $(DEST) $(shell readlink -f $(AM_LIB)) $(shell readlink -f $(KLIB)) $(shell readlink -f $(APP_LIB))

# AM library
$(AM_LIB): $(AM_OBJ)
	ar rcs $(AM_LIB) $(AM_OBJ)
-include $(AM_DEP)

# Kernel library
$(KLIB): $(KLIB_OBJ)
	ar rcs $(KLIB) $(KLIB_OBJ)
-include $(KLIB_DEP)

# Application
$(APP_LIB): $(APP_OBJ)
	ar rcs $(APP_LIB) $(APP_OBJ)
-include $(APP_DEP)

# -----------------------------------------------------------------------------

.PHONY: play clean

play: $(AM_LIB) image
ifeq ($(ARCH), mips32-npc)
	@echo "Burn it to FPGA."
endif
ifeq ($(ARCH), mips32-minimal)
	@echo "Burn it to FPGA."
endif
ifeq ($(ARCH), native)
	@$(DEST)
endif
ifeq ($(ARCH), x86-qemu)
	@qemu-system-i386 -serial stdio $(DEST)
endif

debug: $(AM_LIB) $(DEST)
ifeq ($(ARCH), mips32-npc)
	@echo "Debugging"
endif
ifeq ($(ARCH), native)
	@gdb $(DEST)
endif
ifeq ($(ARCH), x86-qemu)
	@qemu-system-i386 -serial stdio -s -S $(DEST)
endif
clean:
	rm -rf build/ $(shell find . -name "*.o" -o -name "*.d")

'''
