NAME := fceux
SRCS := $(shell find -L ./src/ -name "*.c" -o -name "*.cpp")
ROMS := $(shell find -L ./src/ -name "*.nes")

CFLAGS += -DPSS_STYLE=1
LDFLAGS += -lSDL
#PREBUILD := rom
include $(AM_HOME)/Makefile.app

.PHONY: rom
#rom: $(ROMS)
#	@cd src/roms && python3 build-roms.py
