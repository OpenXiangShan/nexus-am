include $(AM_HOME)/am/arch/isa/riscv64.mk

AM_SRCS := noop/isa/riscv/trm_flash.c \
           nemu/common/mainargs.S \
           noop/isa/riscv/perf.c \
           noop/common/uartlite.c \
           nemu/isa/riscv/trap.S \
           nemu/isa/riscv/vme.c \
           nemu/common/ioe.c \
           noop/common/input.c \
           noop/common/timer.c \
           nemu/common/video.c \
           dummy/audio.c \
           noop/isa/riscv/instr.c \
           xs/isa/riscv/mpe.c \
           xs/isa/riscv/clint.c \
           xs/isa/riscv/pmp.c \
           xs/isa/riscv/plic.c \
           xs/isa/riscv/pma.c \
           xs/isa/riscv/cache.c \
           nemu/isa/riscv/boot/start_flash.S

CFLAGS  += -I$(AM_HOME)/am/src/nemu/include -I$(AM_HOME)/am/src/xs/include -DISA_H=\"riscv.h\"

ASFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/nemu/common/mainargs.S

LDFLAGS += -T $(AM_HOME)/am/src/nemu/isa/riscv/boot/loaderflash.ld

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents --adjust-vma=-0x40000000 -O verilog $(BINARY).elf $(BINARY).bin.txt

NEMU_ARGS = --batch --log=$(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin

run:
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NEMU_ARGS)"
