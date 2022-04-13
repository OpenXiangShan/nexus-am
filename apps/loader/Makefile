NAME = flash-loader
SRCS = main.c image.S
ELF_PATH = /nfs/home/share/xs-workloads/linux-4.18-hello/bbl.bin
ASFLAGS += -DELF_PATH=\"$(ELF_PATH)\"

image.S: $(ELF_PATH)
	touch image.S

include $(AM_HOME)/Makefile.app
