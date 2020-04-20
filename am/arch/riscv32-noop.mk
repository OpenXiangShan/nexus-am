include $(AM_HOME)/am/arch/isa/riscv32.mk

AM_SRCS := noop/isa/riscv/trm.c \
           nemu/common/mainargs.S \
           noop/isa/riscv/perf.c \
           noop/common/uartlite.c \
           nemu/isa/riscv/cte.c \
           nemu/isa/riscv/trap.S \
           nemu/isa/riscv/vme.c \
           nemu/common/ioe.c \
           noop/common/input.c \
           noop/common/timer.c \
           nemu/common/video.c \
           noop/isa/riscv/instr.c \
           dummy/mpe.c \
           nemu/isa/riscv/boot/start.S

CFLAGS  += -I$(AM_HOME)/am/src/nemu/include -DISA_H=\"riscv.h\"

ASFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/nemu/common/mainargs.S

LDFLAGS += -L $(AM_HOME)/am/src/nemu/ldscript
LDFLAGS += -T $(AM_HOME)/am/src/noop/isa/riscv/boot/loader.ld

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:
	$(MAKE) -C $(NOOP_HOME) emu IMAGE="$(BINARY).bin" DATAWIDTH=32
