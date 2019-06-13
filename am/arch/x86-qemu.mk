include $(AM_HOME)/am/arch/isa/x86.mk

AM_SRCS := x86/qemu/trm.c \
           x86/qemu/ioe.c \
           x86/qemu/cte.c \
           x86/qemu/cte_trap.S \
           x86/qemu/vme.c \
           x86/qemu/mpe.c \
           x86/qemu/devices/apic.c \
           x86/qemu/devices/cpu.c \
           x86/qemu/devices/input.c \
           x86/qemu/devices/pcdev.c \
           x86/qemu/devices/timer.c \
           x86/qemu/devices/video.c \

image:
	@make -s -C $(AM_HOME)/am/src/x86/qemu/boot
	@echo + LD "->" $(BINARY_REL).o
	@$(LD) $(LDFLAGS) -Ttext 0x00100000 -o $(BINARY).o $(LINK_FILES)
	@echo + CREATE "->" $(BINARY_REL)
	@( cat $(AM_HOME)/am/src/x86/qemu/boot/mbr; head -c 512 /dev/zero; cat $(BINARY).o ) > $(BINARY)

run:
	@( echo -n $(mainargs); ) | dd if=/dev/stdin of=$(BINARY) bs=512 count=1 seek=1 conv=notrunc status=none
	@qemu-system-i386 -serial stdio -machine accel=kvm:tcg -smp "$(smp)" -drive format=raw,file=$(BINARY)
