NAME := litenes
SRCS := $(shell find -L ./src/ -name "*.c")
ROMS := $(shell find -L ./src/ -name "*.nes")
PREBUILD := src/roms/gen/roms.h
include $(AM_HOME)/Makefile.app

src/roms/gen/roms.h: $(ROMS)
	@cd src/roms && python3 build-roms.py