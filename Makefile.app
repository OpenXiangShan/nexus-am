$(info Building $(NAME) [$(ARCH)])
include $(AM_HOME)/Makefile.check

APP_DIR ?= $(shell pwd)
INC_DIR ?= $(APP_DIR)/include/
DST_DIR ?= $(APP_DIR)/build/$(ARCH)/
ARCHIVE ?= $(APP_DIR)/build/$(NAME)-$(ARCH).a

$(shell mkdir -p $(DST_DIR))

objdest = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(1))))
depdest = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(1))))

OBJS = $(call objdest, $(SRCS))
DEPS = $(call depdest, $(SRCS))

$(ARCHIVE): $(OBJS)
	ar rcs $(ARCHIVE) $(OBJS)

include $(AM_HOME)/Makefile.compile


.PHONY: image clean
image: $(ARCHIVE)$(OBJS)
	@echo $(OBJS)
	@echo "Hello"

clean:
	rm -rf $(APP_DIR)/build/

