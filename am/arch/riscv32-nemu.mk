AM_SRCS := riscv32/nemu/trm.c \
           riscv32/nemu/ioe.c \
           riscv32/nemu/trap.S \
           riscv32/nemu/cte.c \
           riscv32/nemu/vme.c \
           nemu-devices/nemu-input.c \
           nemu-devices/nemu-timer.c \
           nemu-devices/nemu-video.c \

LOADER_DIR := $(AM_HOME)/am/src/risv32/nemu/loader

image:
	@echo + CC loader/start.S
	@riscv-none-embed-gcc -march=rv32im -mabi=ilp32 -fno-pic -c $(LOADER_DIR)/start.S -o $(LOADER_DIR)/start.o
	@echo + LD "->" $(BINARY).o
	@riscv-none-embed-ld --gc-sections -melf32lriscv -T $(LOADER_DIR)/loader.ld -e _start -o $(BINARY).o $(LOADER_DIR)/start.o --start-group $(LINK_FILES) --end-group
	@riscv-none-embed-objdump -d $(BINARY).o > $(BINARY).txt
	@echo + CREATE "->" $(BINARY).bin
	@riscv-none-embed-objcopy -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).o $(BINARY).bin

run:
	make -C $(NEMU_HOME) ISA=riscv32 run ARGS="-b -l $(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin"