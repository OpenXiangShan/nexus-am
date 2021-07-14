include $(AM_HOME)/am/arch/isa/riscv64.mk

AM_SRCS := nutshell/isa/riscv/trm.c \
           nutshell/common/mainargs.S \
           nutshell/common/uartlite.c \
           nutshell/common/ioe.c \
           nutshell/common/timer.c \
           dummy/input.c \
           dummy/video.c \
           dummy/audio.c \
           dummy/cte.c \
           dummy/vme.c \
           dummy/mpe.c \
           nutshell/isa/riscv/boot/start.S

CFLAGS  += -I$(AM_HOME)/am/src/nutshell/include -DISA_H=\"riscv.h\"

ASFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/nutshell/common/mainargs.S

LDFLAGS += -L $(AM_HOME)/am/src/nutshell/ldscript
LDFLAGS += -T $(AM_HOME)/am/src/nutshell/isa/riscv/boot/loader64.ld

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -o $(BINARY).elf --start-group $(LINK_FILES) --end-group
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:
	$(MAKE) -C $(NOOP_HOME) emu-run IMAGE="$(BINARY).bin" DATAWIDTH=64
