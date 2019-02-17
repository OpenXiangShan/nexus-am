AM_SRCS := x86/qemu/trm.c \
           x86/qemu/ioe.c \
           x86/qemu/cte.c \
           x86/qemu/cte_trap.S \
           x86/qemu/vme.c \
           x86/qemu/mpe.c \
           x86/qemu/trace.c \
           x86/qemu/devices/apic.c \
           x86/qemu/devices/cpu.c \
           x86/qemu/devices/input.c \
           x86/qemu/devices/pcdev.c \
           x86/qemu/devices/timer.c \
           x86/qemu/devices/video.c \

image:
	@ld -melf_i386 -Ttext 0x00100000 -o $(BINARY).o --start-group $(LINK_FILES) --end-group 
	@make -C ${AM_HOME}/am/src/x86/qemu/boot
	@cat ${AM_HOME}/am/src/x86/qemu/boot/mbr $(BINARY).o > $(BINARY)

run:
	@qemu-system-i386 -serial stdio $(BINARY)