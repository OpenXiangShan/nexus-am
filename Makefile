.DEFAULT_GOAL = library
AM_HOME ?= $(shell pwd)

$(shell mkdir -p build/$(ARCH))

# -----------------------------------------------------------------------------

findsrc = $(shell find -L $(1) -name "*.c" -o -name "*.cpp" -o -name "*.S")
objdest = $(addprefix build/$(ARCH)/, $(addsuffix .o, $(basename $(1))))
depdest = $(addprefix build/$(ARCH)/, $(addsuffix .d, $(basename $(1))))

# AM library archive
AM_PATH = ./am/arch/$(ARCH)
AM_LIB  = ./build/$(ARCH)/libam.a
AM_SRC  = $(call findsrc, $(AM_PATH)/src)
AM_OBJ  = $(call objdest, $(AM_SRC))
AM_DEP  = $(call depdest, $(AM_SRC))

include Makefile.compile

# -----------------------------------------------------------------------------

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

.PHONY: library
library: $(AM_OBJ)
	ar rcs $(AM_LIB) $(AM_OBJ)
-include $(AM_DEP)

# -----------------------------------------------------------------------------

.PHONY: play clean
clean:
	rm -rf build/ $(shell find . -name "*.o" -o -name "*.d")

