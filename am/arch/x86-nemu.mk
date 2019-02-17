AM_SRCS := x86/nemu/trm.c \
           x86/nemu/ioe.c \
           x86/nemu/cte.c \
           x86/nemu/trap.S \
           x86/nemu/vme.c \
           nemu-devices/nemu-input.c \
           nemu-devices/nemu-timer.c \
           nemu-devices/nemu-video.c \

LOADER_DIR := $(AM_HOME)/am/src/x86/nemu/loader

image:
	@echo + CC loader/start.S
	@gcc -m32 -fno-pic -ffunction-sections -c $(LOADER_DIR)/start.S -o $(LOADER_DIR)/start.o
	@echo + LD "->" $(BINARY).o
	@ld -melf_i386 --gc-sections -T $(LOADER_DIR)/loader.ld -e _start -o $(BINARY).o $(LOADER_DIR)/start.o --start-group $(LINK_FILES) --end-group
	@objdump -d $(BINARY).o > $(BINARY).txt
	@echo + CREATE "->" $(BINARY).bin
	@objcopy -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).o $(BINARY).bin

run:
	make -C $(NEMU_HOME) ISA=x86 run ARGS="-b -l $(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin"	