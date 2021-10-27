NAME = flash-loader
SRCS = main.c image.S
#ELF_PATH = /home/yzh/projectn/xv6-riscv/kernel/kernel
#ELF_PATH = /home/yzh/projectn/nexus-am/apps/hello/build/hello-riscv64-noop.elf
#ELF_PATH = /home/yzh/projectn/nexus-am/apps/coremark/build/coremark-riscv64-noop.elf
ELF_PATH = /home/jy/Project/nexus-am/tests/cputest/build/add-riscv64-xs.bin
ASFLAGS += -DELF_PATH=\"$(ELF_PATH)\"

image.S: $(ELF_PATH)
	touch image.S

include $(AM_HOME)/Makefile.app
