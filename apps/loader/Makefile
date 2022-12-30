NAME = flash-loader
SRCS = main.c image.S
ELF_PATH ?= ./image/linux-hello-256M.bin
ASFLAGS += -DELF_PATH=\"$(ELF_PATH)\"

image.S: $(ELF_PATH)
	touch image.S

include $(AM_HOME)/Makefile.app
