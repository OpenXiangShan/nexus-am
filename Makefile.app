.DEFAULT_GOAL = app

include $(AM_HOME)/Makefile.check
$(info Building $(NAME) [$(ARCH)] with AM_HOME {$(AM_HOME)})

APP_DIR ?= $(shell pwd)
INC_DIR += $(APP_DIR)/include/
DST_DIR ?= $(APP_DIR)/build/$(ARCH)/
BINARY ?= $(APP_DIR)/build/$(NAME)-$(ARCH)

LIBS += klib compiler-rt

INC_DIR += $(addsuffix /include/, $(addprefix $(AM_HOME)/libs/, $(LIBS)))

$(shell mkdir -p $(DST_DIR))

include $(AM_HOME)/Makefile.compile

LINKLIBS = $(LIBS)

LINK_FILES += $(AM_HOME)/am/build/am-$(ARCH).a $(OBJS)
LINK_FILES += $(addsuffix -$(ARCH).a, $(join \
  $(addsuffix /build/, $(addprefix $(AM_HOME)/libs/, $(LINKLIBS))), \
  $(LINKLIBS) \
))

.PHONY: app run clean
app: $(OBJS) am $(LIBS)
	@bash $(AM_HOME)/am/arch/$(ARCH)/img/build $(BINARY) $(LINK_FILES)
run: app
	@bash $(AM_HOME)/am/arch/$(ARCH)/img/run $(BINARY)

clean: 
	rm -rf $(APP_DIR)/build/
