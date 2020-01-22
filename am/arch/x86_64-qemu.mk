include $(AM_HOME)/am/arch/isa/x86_64.mk

AM_SRCS   := x86_64/qemu/start.S \
             x86_64/qemu/trm.c \

LD_SCRIPT := $(AM_HOME)/am/src/x86_64/qemu/boot/loader.ld

image:
	@make -s -C $(AM_HOME)/am/src/x86_64/qemu/boot
	@echo + LD "->" $(BINARY_REL).o
	@$(LD) $(LDFLAGS) -Ttext-segment 0 -o $(BINARY).o $(LINK_FILES)
	@echo + CREATE "->" $(BINARY_REL)
	@( cat $(AM_HOME)/am/src/x86_64/qemu/boot/mbr; head -c 512 /dev/zero; cat $(BINARY).o ) > $(BINARY)

run:
	@( echo -n $(mainargs); ) | dd if=/dev/stdin of=$(BINARY) bs=512 count=1 seek=1 conv=notrunc status=none
	@qemu-system-x86_64 -serial stdio -machine accel=kvm:tcg -smp "$(smp)" -drive format=raw,file=$(BINARY)

debug:
	@( echo -n $(mainargs); ) | dd if=/dev/stdin of=$(BINARY) bs=512 count=1 seek=1 conv=notrunc status=none
	@qemu-system-x86_64 -S -s -serial none -machine accel=kvm:tcg -smp "$(smp)" -drive format=raw,file=$(BINARY) -nographic # & pid=$$!; gdb -x x.gdb && kill -9 $$pid
