.DEFAULT_GOAL = app

$(info Building $(NAME) [$(ARCH)])
include $(AM_HOME)/Makefile.check

APP_DIR ?= $(shell pwd)
INC_DIR += $(APP_DIR)/include/ $(AM_HOME)/klib/
DST_DIR ?= $(APP_DIR)/build/$(ARCH)/
BINARY ?= $(APP_DIR)/build/$(NAME)-$(ARCH)

$(shell mkdir -p $(DST_DIR))

include $(AM_HOME)/Makefile.compile

LINK_FILES = $(AM_HOME)/am/build/am-$(ARCH).a $(OBJS)

ifneq ($(ARCH), native)
LINK_FILES += $(AM_HOME)/klib/build/klib-$(ARCH).a
endif

.PHONY: app run clean
app: $(OBJS)
	@cd $(AM_HOME) && make ARCH=$(ARCH)
	@$(AM_HOME)/am/arch/$(ARCH)/img/build $(BINARY) $(LINK_FILES)
run: app
	@$(AM_HOME)/am/arch/$(ARCH)/img/run $(BINARY)

clean: 
	rm -rf $(APP_DIR)/build/
