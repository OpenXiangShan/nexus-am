include $(AM_HOME)/am/arch/isa/riscv32.mk

AM_SRCS := $(ISA)/noop/trm.c \
           $(ISA)/noop/uartlite.c \
           $(ISA)/noop/perf.c \
           $(ISA)/noop/cte.c \
           $(ISA)/nemu/trap.S \
           $(ISA)/noop/instr.c \
           $(ISA)/nemu/vme.c \
           nemu-common/ioe.c \
           $(ISA)/noop/input.c \
           nemu-common/nemu-timer.c \
           nemu-common/nemu-video.c \
           dummy/mpe.c \
           $(ISA)/nemu/boot/start.S

LD_SCRIPT     := $(AM_HOME)/am/src/$(ISA)/nemu/boot/loader.ld

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -T $(LD_SCRIPT) -e _start -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:
	$(MAKE) -C $(NOOP_HOME) emu IMAGE="$(BINARY).bin"
