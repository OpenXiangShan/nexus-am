include $(AM_HOME)/am/arch/isa/riscv32.mk

AM_SRCS := $(ISA)/noop/trm.c \
           $(ISA)/noop/uartlite.c \
           $(ISA)/noop/perf.c \
           $(ISA)/noop/cte.c \
           $(ISA)/nemu/trap.S \
           $(ISA)/noop/instr.c \
           $(ISA)/nemu/vme.c \
           nemu-devices/ioe.c \
           nemu-devices/nemu-input.c \
           nemu-devices/nemu-timer.c \
           nemu-devices/nemu-video.c \
           $(ISA)/nemu/boot/start.S

LD_SCRIPT     := $(AM_HOME)/am/src/$(ISA)/nemu/boot/loader.ld

GEN_READMEMH := $(NOOP_HOME)/tools/readmemh/build/verilator-readmemh
$(GEN_READMEMH):
	$(MAKE) -C $(@D)

image: $(GEN_READMEMH)
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -T $(LD_SCRIPT) -e _start -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL)-readmemh
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O verilog --adjust-vma -0x80000000 $(BINARY).elf $(BINARY)-readmemh
	@$(GEN_READMEMH) $(BINARY)-readmemh

run:
	make -C $(NOOP_HOME) emu IMAGE="$(BINARY)-readmemh"
