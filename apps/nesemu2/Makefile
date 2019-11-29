NAME := nesemu2
SRCS := $(shell find -L ./src/ -name "*.c")
CFLAGS += -DCPU_UNDOC -DQUICK_SPRITES -DSDL -DLINUX -DRELEASE -DNO_FILE_SYSTEM -Isrc

include $(AM_HOME)/Makefile.app

.PHONY: rom
rom:
	@cd src/roms && python3 build-roms.py
