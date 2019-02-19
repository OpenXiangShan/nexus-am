AM_SRCS := $(ISA)/nemu/trm.c \
           $(ISA)/nemu/cte.c \
           $(ISA)/nemu/trap.S \
           $(ISA)/nemu/vme.c \
           nemu-devices/ioe.c \
           nemu-devices/nemu-input.c \
           nemu-devices/nemu-timer.c \
           nemu-devices/nemu-video.c \
           $(ISA)/nemu/boot/start.S

LDFLAGS := -melf32lriscv
LD_SCRIPT := $(AM_HOME)/am/src/$(ISA)/nemu/boot/loader.ld

GEN_READMEMH := $(NOOP_HOME)/tools/readmemh/verilator-readmemh
$(GEN_READMEMH):
	$(MAKE) -C (@D)

image: $(GEN_READMEMH)
	@echo + LD "->" $(BINARY).elf
	@$(LD) $(LDFLAGS) --gc-sections -T $(LD_SCRIPT) -e _start -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY)-readmemh
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O verilog --adjust-vma -0x80000000 $(BINARY).elf $(BINARY)-readmemh
	@$(GEN_READMEMH) $(BINARY)-readmemh

run:
	make -C $(NOOP_HOME) emu IMAGE="$(BINARY_ABS)-readmemh"
