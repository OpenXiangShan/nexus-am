APP_DIR ?= $(shell pwd)
SRC_DIR ?= $(APP_DIR)/src/
INC_DIR ?= $(APP_DIR)/include/
DST_DIR ?= $(APP_DIR)/build/

$(shell mkdir -p $(DST_DIR))

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

.PHONY: image
image: $(OBJS)
	@echo "Hello"
