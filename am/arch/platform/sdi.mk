AM_SRCS += sdi/trm.c \
           nemu/common/mainargs.S \
           nemu/common/ioe.c \
           nemu/common/input.c \
           nemu/common/timer.c \
           nemu/common/video.c \
           dummy/audio.c \
           dummy/cte.c \
           dummy/vme.c \
           dummy/mpe.c \

CFLAGS  += -I$(AM_HOME)/am/src/sdi/include
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
	$(MAKE) -C $(NEMU_HOME) ISA=riscv64 SHARE=1
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) ENGINE=rv64 run ARGS="$(NEMU_ARGS)"

#run:
#	$(MAKE) -C $(NEMU_HOME) ISA=riscv64 ENGINE=sdi-exec run ARGS="--batch $(BINARY).bin" &
#	( while ps -p $$PPID >/dev/null ; do sleep 1 ; done ; pkill -9 -f riscv64-nemu-sdi-exec ) &
#	sleep 3 # wait for riscv64-nemu-sdi-exec to start
#	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) ENGINE=sdi-tran run ARGS="$(NEMU_ARGS)"

gdb: image
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) ENGINE=rv64 gdb ARGS="$(NEMU_ARGS)"
