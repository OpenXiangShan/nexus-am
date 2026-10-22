APP_DIR := $(CURDIR)
MT_MAKEFILE := $(abspath $(firstword $(MAKEFILE_LIST)))
LOW_MAINARGS ?= help
STRESS_MAINARGS ?= queue

ifeq ($(MT_VARIANT),low)
NAME = mt-low
SRCS = src/common.c src/low_main.c
include $(AM_HOME)/Makefile.app
else ifeq ($(MT_VARIANT),stress)
NAME = mt-stress
SRCS = src/common.c src/stress_main.c
include $(AM_HOME)/Makefile.app
else
.PHONY: all low stress clean

all: low stress

low:
	@$(MAKE) -f $(MT_MAKEFILE) MT_VARIANT=low mainargs='$(LOW_MAINARGS)'

stress:
	@$(MAKE) -f $(MT_MAKEFILE) MT_VARIANT=stress mainargs='$(STRESS_MAINARGS)'

clean:
	rm -rf $(APP_DIR)/build/
endif
