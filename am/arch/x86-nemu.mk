AM_SRCS := x86/nemu/trm.c \
           x86/nemu/ioe.c \
           x86/nemu/cte.c \
           x86/nemu/trap.S \
           x86/nemu/vme.c \
           x86/nemu/devices/input.c \
           x86/nemu/devices/timer.c \
           x86/nemu/devices/video.c \

DIR := $(AM_HOME)/am/src/x86/nemu/loader

image:
	gcc -m32 -fno-pic -ffunction-sections -c $(DIR)/start.S -o $(DIR)/start.o
	ld -melf_i386 --gc-sections -T $(DIR)/loader.ld -e _start -o $(BINARY).o $(DIR)/start.o --start-group $(LINK_FILES) --end-group
	objdump -d $(BINARY).o > $(BINARY).txt
	objcopy -S --set-section-flags .bss=alloc,contents -O binary $(BINARY) $(BINARY).bin

run:
	make -C $(NEMU_HOME) ISA=x86 run ARGS="-b -l $(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin"	