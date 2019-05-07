AM_SRCS := $(ISA)/nemu/trm.c \
           $(ISA)/nemu/cte.c \
           $(ISA)/nemu/trap.S \
           $(ISA)/nemu/vme.c \
           nemu-devices/ioe.c \
           nemu-devices/nemu-input.c \
           nemu-devices/nemu-timer.c \
           nemu-devices/nemu-video.c \
           $(ISA)/nemu/boot/start.S

LD_SCRIPT := $(AM_HOME)/am/src/$(ISA)/nemu/boot/loader.ld

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -T $(LD_SCRIPT) -e _start -o $(BINARY).elf $(LINK_FILES)
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:
	make -C $(NEMU_HOME) ISA=$(ISA) run ARGS="-b -l $(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin"

gdb: image
	make -C $(NEMU_HOME) ISA=$(ISA) gdb ARGS="-b -l $(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin"
