AM_SRCS := $(ISA)/nemu/trm.c \
           $(ISA)/nemu/ioe.c \
           $(ISA)/nemu/cte.c \
           $(ISA)/nemu/trap.S \
           $(ISA)/nemu/vme.c \
           nemu-devices/nemu-input.c \
           nemu-devices/nemu-timer.c \
           nemu-devices/nemu-video.c \
           $(ISA)/nemu/boot/start.S

LD_SCRIPT := $(AM_HOME)/am/src/$(ISA)/nemu/boot/loader.ld

image:
	@echo + LD "->" $(BINARY).o
	@ld -melf_i386 --gc-sections -T $(LD_SCRIPT) -e _start -o $(BINARY).o --start-group $(LINK_FILES) --end-group
	@objdump -d $(BINARY).o > $(BINARY).txt
	@echo + CREATE "->" $(BINARY).bin
	@objcopy -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).o $(BINARY).bin

run:
	make -C $(NEMU_HOME) ISA=$(ISA) run ARGS="-b -l $(shell dirname $(BINARY_ABS))/nemu-log.txt $(BINARY_ABS).bin"
