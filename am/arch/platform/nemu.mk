AM_SRCS := nemu-common/trm.c \
           nemu-common/ioe.c \
           nemu-common/nemu-input.c \
           nemu-common/nemu-timer.c \
           nemu-common/nemu-video.c \
           $(ISA)/nemu/cte.c \
           $(ISA)/nemu/trap.S \
           $(ISA)/nemu/vme.c \
           dummy/mpe.c \
           $(ISA)/nemu/boot/start.S

LDFLAGS += -L $(AM_HOME)/am/src/nemu-common
LDFLAGS += -T $(AM_HOME)/am/src/$(ISA)/nemu/boot/loader.ld

ifdef mainargs
MAINARGS = --mainargs=$(mainargs)
endif
NEMU_ARGS = --batch $(MAINARGS) --log=$(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -o $(BINARY).elf $(LINK_FILES)
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NEMU_ARGS)"

gdb: image
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) gdb ARGS="$(NEMU_ARGS)"
