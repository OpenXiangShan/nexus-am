include $(AM_HOME)/am/arch/isa/riscv64.mk

AM_SRCS := xs/isa/riscv/trm.c \
           xs/common/mainargs.S \
           xs/isa/riscv/perf.c \
           xs/common/uartlite.c \
           xs/isa/riscv/cte.c \
           xs/isa/riscv/trap.S \
           xs/isa/riscv/cte64.c \
           xs/isa/riscv/mtime.S \
           xs/isa/riscv/vme.c \
           xs/common/ioe.c \
           xs/common/input.c \
           xs/common/timer.c \
           xs/common/video.c \
           xs/common/audio.c \
           xs/isa/riscv/instr.c \
           xs/isa/riscv/mpe.c \
           xs/isa/riscv/clint.c \
           xs/isa/riscv/pmp.c \
           xs/isa/riscv/plic.c \
           xs/isa/riscv/pma.c \
           xs/isa/riscv/cache.c \
           nemu/isa/riscv/boot/start.S

CFLAGS  += -I$(AM_HOME)/am/src/nemu/include -I$(AM_HOME)/am/src/xs/include -DISA_H=\"riscv.h\"

ASFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/nemu/common/mainargs.S

LDFLAGS += -L $(AM_HOME)/am/src/nemu/ldscript
LDFLAGS += -T $(AM_HOME)/am/src/nemu/isa/riscv/boot/loader64.ld

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:
	$(MAKE) -C $(NOOP_HOME) emu-run IMAGE="$(BINARY).bin" DATAWIDTH=64
