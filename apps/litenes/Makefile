NAME := litenes

ROM_PATH := $(AM_HOME)/share/games/nes
ROM_SRC  := $(ROM_PATH)/gen/mario.c

SRCS := $(shell find -L ./src/ -name "*.c") $(ROM_SRC)
include $(AM_HOME)/Makefile.app

$(ROM_SRC):
	@make -C $(ROM_PATH)
