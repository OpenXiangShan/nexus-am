NAME := amtest
SRCS := $(shell find -L ./src/ -name "*.[cS]")

ifeq ($(ARCH),riscv64-nutshell)
SRCS := ./src/main.c \
        ./src/tests/hello.c \
        ./src/tests/devscan.c \
        ./src/tests/rtc.c \
        ./src/tests/keyboard.c \
        ./src/tests/video.c \
        ./src/tests/audio.c \
        ./src/tests/audio/audio-data.S
endif

include $(AM_HOME)/Makefile.app
