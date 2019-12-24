include $(AM_HOME)/am/arch/isa/riscv64.mk

CFLAGS += -march=rv64im

AM_SRCS := $(ISA)/asic/trm.c \
           $(ISA)/asic/uartlite.c \
           $(ISA)/noop/cte.c \
           $(ISA)/noop/trap.S \
           $(ISA)/noop/instr.c \
           $(ISA)/noop/vme.c \
           nemu-common/ioe.c \
           $(ISA)/noop/input.c \
           nemu-common/nemu-timer.c \
           nemu-common/nemu-video.c \
           dummy/mpe.c \
           $(ISA)/asic/boot/start.S

LDFLAGS += -T $(AM_HOME)/am/src/$(ISA)/asic/boot/loader.ld

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O verilog $(BINARY).elf $(BINARY).bin.txt

run:
	$(MAKE) -C $(NOOP_HOME) emu IMAGE="$(BINARY).bin"
