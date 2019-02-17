AM_SRCS := x86/x86-qemu-trm.c \
           x86/x86-qemu-ioe.c \
           x86/x86-qemu-cte.c \
           x86/x86-qemu-cte_trap.S \
           x86/x86-qemu-vme.c \
           x86/x86-qemu-mpe.c \
           x86/x86-qemu-trace.c \
           x86/devices/x86-apic.c \
           x86/devices/x86-qemu-cpu.c \
           x86/devices/x86-qemu-input.c \
           x86/devices/x86-qemu-pcdev.c \
           x86/devices/x86-qemu-timer.c \
           x86/devices/x86-qemu-video.c \

image:
	@ld -melf_i386 -Ttext 0x00100000 -o $(BINARY).o --start-group $(LINK_FILES) --end-group 
	@make -C ${AM_HOME}/am/src/x86/boot
	@cat ${AM_HOME}/am/src/x86/boot/mbr $(BINARY).o > $(BINARY)

run:
	@qemu-system-i386 -serial stdio $(BINARY)