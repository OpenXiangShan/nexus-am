CC = gcc
LD = ld
CFLAGS = -m32 -march=i386 -static -MD -std=gnu89 -ggdb \
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
		 -Wall -Werror -O2
ASFLAGS = -ggdb -m32 -MD
LDFLAGS = -melf_i386
QEMU = qemu-system-i386

x86-arch:
	@cd boot; make
	@cd kernel; make
	@cd ../../build; rm -rf x86; mkdir x86
	cat boot/mbr kernel/kmain > ../../build/x86/x86-arch

clean:
	@cd boot; make clean
	@cd kernel; make clean
	rm -rf ../../build/x86
