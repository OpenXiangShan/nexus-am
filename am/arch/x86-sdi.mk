include $(AM_HOME)/am/arch/isa/x86.mk

AM_SRCS += sdi/trm.c \
           nemu/common/mainargs.S \
           nemu/isa/x86/boot/start.S \
           nemu/common/ioe.c \
           nemu/common/input.c \
           nemu/common/timer.c \
           nemu/common/video.c \
           dummy/mpe.c \

CFLAGS  += -mstringop-strategy=loop -I$(AM_HOME)/am/src/sdi/include
ASFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/nemu/common/mainargs.S

LDFLAGS += -L $(AM_HOME)/am/src/nemu/ldscript
LDFLAGS += -T $(AM_HOME)/am/src/sdi/ldscript/loader.ld

NEMU_ARGS = --batch --log=$(shell dirname $(BINARY))/nemu-log.txt $(BINARY).bin

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
