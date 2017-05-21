.DEFAULT_GOAL = app

$(info Building $(NAME) [$(ARCH)])
include $(AM_HOME)/Makefile.check

APP_DIR ?= $(shell pwd)
INC_DIR += $(APP_DIR)/include/ $(AM_HOME)/klib/
DST_DIR ?= $(APP_DIR)/build/$(ARCH)/
BINARY ?= $(APP_DIR)/build/$(NAME)-$(ARCH)

$(shell mkdir -p $(DST_DIR))

include $(AM_HOME)/Makefile.compile

.PHONY: app clean
app: $(OBJS)
	@cd $(AM_HOME) && make ARCH=$(ARCH)
	@$(AM_HOME)/am/arch/$(ARCH)/img/build $(BINARY) $(AM_HOME)/am/build/am-$(ARCH).a $(AM_HOME)/klib/build/klib-$(ARCH).a $(OBJS)

clean: 
	rm -rf $(APP_DIR)/build/
