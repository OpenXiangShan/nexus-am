AM_SRCS := mips32/nemu/trm.c \
           mips32/nemu/ioe.c \
           mips32/nemu/trap.S \
           mips32/nemu/cte.c \
           mips32/nemu/vme.c \
           devices/nemu-input.c \
           devices/nemu-timer.c \
           devices/nemu-video.c \

LOADER_DIR := $(AM_HOME)/am/src/mips32/nemu/loader

image:
	mips-linux-gnu-gcc -EL -march=mips32 -fno-pic -mno-abicalls -fno-delayed-branch -c $(LOADER_DIR)/start.S -o $(LOADER_DIR)/start.o
	mips-linux-gnu-ld --gc-sections -EL -T $(LOADER_DIR)/loader.ld -e _start -o $(BINARY).o $(LOADER_DIR)/start.o --start-group $(LINK_FILES) --end-group
	mips-linux-gnu-objdump -d $(BINARY).o > $(BINARY).txt
	mips-linux-gnu-objcopy -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).o $(BINARY).bin

run:
	make -C $(NEMU_HOME) ISA=mips32 run ARGS="-b -l $(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin"