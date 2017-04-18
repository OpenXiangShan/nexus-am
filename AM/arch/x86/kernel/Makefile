CC = gcc
LD = ld
CFLAGS = -m32 -march=i386 -static -MD -std=gnu89 -ggdb \
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
		 -Wall -Werror -O2 -I./include
ASFLAGS = -ggdb -m32 -MD
LDFLAGS = -melf_i386
QEMU = qemu-system-i386

# 编译目标：src目录下的所有.c和.S文件
KCFILES = $(shell find ./ -name "*.c")
KSFILES = $(shell find ./ -name "*.S")
KOBJS = $(KCFILES:.c=.o) $(KSFILES:.S=.o)

kmain: $(KOBJS)
	$(LD) $(LDFLAGS) -e kentry -Ttext 0x00100000 -o kmain $(KOBJS)
	./genkernel.pl kmain
	
-include $(patsubst %.o, %.d, $(KOBJS))

clean:
	rm -rf $(KOBJS) $(KOBJS:.o=.d) kmain
