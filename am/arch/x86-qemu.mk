include $(AM_HOME)/am/arch/isa/x86.mk

AM_SRCS   := x86/qemu/start32.S \
             x86/qemu/trap32.S \
             x86/qemu/trm.c \
             x86/qemu/cte.c \
             x86/qemu/ioe.c \
             x86/qemu/vme.c \
             x86/qemu/mpe.c

image:
	@make -s -C $(AM_HOME)/am/src/x86/qemu/boot
	@echo + LD "->" $(BINARY_REL).o
	@$(LD) $(LDFLAGS) -Ttext 0x00100000 -o $(BINARY).o $(LINK_FILES)
	@echo + CREATE "->" $(BINARY_REL)
	@( cat $(AM_HOME)/am/src/x86/qemu/boot/mbr; head -c 1024 /dev/zero; cat $(BINARY).o ) > $(BINARY)

run:
	@( echo -n $(mainargs); ) | dd if=/dev/stdin of=$(BINARY) bs=512 count=2 seek=1 conv=notrunc status=none
	@qemu-system-i386 -serial stdio -machine accel=tcg -smp "$(smp)" -drive format=raw,file=$(BINARY)
