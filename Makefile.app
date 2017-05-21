APP_DIR ?= $(shell pwd)
SRC_DIR ?= $(APP_DIR)/src/
INC_DIR ?= $(APP_DIR)/include/
DST_DIR ?= $(APP_DIR)/build/

$(shell mkdir -p $(DST_DIR))

objdest = $(addprefix build/$(ARCH)/, $(addsuffix .o, $(basename $(1))))
depdest = $(addprefix build/$(ARCH)/, $(addsuffix .o, $(basename $(1))))

OBJS = $(objdest $(SRCS))
DEPS = $(objdest $(DEPS))

include ${AM_HOME}/Makefile.compile

# Compilation patterns
$(DST_DIR)/$(ARCH)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DST_DIR)/$(ARCH)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(DST_DIR)/$(ARCH)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c -o $@ $<

.PHONY: image clean
image: $(OBJS)
	@echo "Hello"

clean:
	rm -rf $(APP_DIR)/build/

