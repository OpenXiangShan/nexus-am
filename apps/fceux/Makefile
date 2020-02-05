NAME := fceux

ROM_PATH := $(AM_HOME)/share/games/nes
ROM_SRC := $(shell find -L $(ROM_PATH)/gen/ -name "*.c")
INCLUDES += -I$(ROM_PATH)/gen/
SRCS := $(shell find -L ./src/ -name "*.c" -o -name "*.cpp") $(ROM_SRC)

CFLAGS += -DPSS_STYLE=1 -DFRAMESKIP -D__NO_FILE_SYSTEM__
include $(AM_HOME)/Makefile.app

.PHONY: rom
rom: $(ROMS)
	@cd $(ROM_PATH) && python3 build-roms.py
