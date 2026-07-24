MODE ?= sv39
HALF_RVI ?= 0
VALID_MODES := sv39 sv48 sv57 sv39x4 sv48x4

ifneq ($(words $(MODE)),1)
$(error MODE must select exactly one of: $(VALID_MODES))
endif
ifeq ($(filter $(MODE),$(VALID_MODES)),)
$(error Invalid MODE="$(MODE)". Supported modes: $(VALID_MODES))
endif
ifneq ($(words $(HALF_RVI)),1)
$(error HALF_RVI must be either 0 or 1)
endif
ifeq ($(filter 0 1,$(HALF_RVI)),)
$(error Invalid HALF_RVI="$(HALF_RVI)". Supported values: 0 1)
endif

SUFFIX := $(MODE)$(if $(filter 1,$(HALF_RVI)),-half-rvi,)
NAME = fetch-fall-canonical-test-$(SUFFIX)
SRCS = main.c
CPPFLAGS += -DMODE=$(MODE) -DHALF_RVI=$(HALF_RVI)
DST_DIR = $(APP_DIR)/build/$(ARCH)/$(SUFFIX)/

include $(AM_HOME)/Makefile.app
