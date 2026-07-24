MODE ?= sv39
HALF_RVI ?= 0
JUMP ?= 0
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
ifneq ($(words $(JUMP)),1)
$(error JUMP must select exactly one transfer type)
endif
ifeq ($(filter 0 1 2 3 4,$(JUMP)),)
$(error Invalid JUMP="$(JUMP)". Supported values: 0 1 2 3 4)
endif
ifneq ($(filter 1,$(HALF_RVI)),)
ifneq ($(filter-out 0,$(JUMP)),)
$(error HALF_RVI=1 is valid only when JUMP=0)
endif
endif

JUMP_SUFFIX_0 :=
JUMP_SUFFIX_1 := -beqz
JUMP_SUFFIX_2 := -jal
JUMP_SUFFIX_3 := -jalr
JUMP_SUFFIX_4 := -mret
SUFFIX := $(MODE)$(JUMP_SUFFIX_$(JUMP))$(if $(filter 1,$(HALF_RVI)),-half-rvi,)
NAME = fetch-canonical-check-test-$(SUFFIX)
SRCS = main.c
CPPFLAGS += -DMODE=$(MODE) -DHALF_RVI=$(HALF_RVI) -DJUMP=$(JUMP)
DST_DIR = $(APP_DIR)/build/$(ARCH)/$(SUFFIX)/

include $(AM_HOME)/Makefile.app
