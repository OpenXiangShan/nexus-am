.DEFAULT_GOAL = app

include $(AM_HOME)/Makefile.check
-include $(AM_HOME)/am/arch/$(ARCH).mk
$(info Building $(NAME) [$(ARCH)] with AM_HOME {$(AM_HOME)})

APP_DIR ?= $(shell pwd)
INC_DIR += $(APP_DIR)/include/
DST_DIR ?= $(APP_DIR)/build/$(ARCH)/
BINARY  ?= $(shell realpath $(APP_DIR)/build/$(NAME)-$(ARCH) --relative-to .)
BINARY_ABS = $(abspath $(BINARY))

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

.PHONY: app run image prompt

$(OBJS): $(PREBUILD)
image: $(OBJS) am $(LIBS) prompt
prompt: $(OBJS) am $(LIBS)
app: image
run: app

prompt:
	@echo Creating binary image [$(ARCH)]

clean: 
	rm -rf $(APP_DIR)/build/
