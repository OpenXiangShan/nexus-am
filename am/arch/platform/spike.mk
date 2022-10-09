AM_SRCS += nemu/common/trm.c \
           nemu/common/mainargs.S \
           nemu/common/ioe.c \
           nemu/common/input.c \
           nemu/common/timer.c \
           nemu/common/video.c \
           nemu/common/audio.c \
           dummy/mpe.c \

CFLAGS  += -I$(AM_HOME)/am/src/nemu/include
ASFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/nemu/common/mainargs.S

LDFLAGS += -L $(AM_HOME)/am/src/spike/ldscript

NEMU_ARGS +=  -l --isa=$(RV_MARCH) 
ifeq ($(HAS_RVV),1)
NEMU_ARGS +=  --varch=vlen:256,elen:64 
endif
NEMU_ARGS +=  $(BINARY).elf 2> $(BINARY).log

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) --gc-sections -o $(BINARY).elf $(LINK_FILES)
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:image
	spike $(NEMU_ARGS)

gdb: image
	spike -d  $(NEMU_ARGS)
