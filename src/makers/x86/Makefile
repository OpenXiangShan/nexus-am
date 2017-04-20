ARCH = x86
CC = gcc
LD = ld
CFLAGS = -m32 -march=i386 -static -MD -std=gnu89 -ggdb -g\
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
		 -Wall -Werror -O2 -I../../../include/common -I../../../include/$(ARCH)
ASFLAGS = -ggdb -m32 -MD
LDFLAGS = -melf_i386

OBJ_DIR = obj
CSRC = $(shell find  ../../test/ -name "*.c")
SSRC = $(shell find  ../../test/ -name "*.S")
LIBS = ../../../Lib/$(ARCH)/libam-$(ARCH).a
OBJECTS = $(SSRC:%.S=$(OBJ_DIR)/%.o) $(CSRC:%.c=$(OBJ_DIR)/%.o)
DOT_D = $(SSRC:.S=.d) $(CSRC:.c=.d)
ASMS = $(CSRC:.c=.S)

umain: $(OBJECTS)
	$(LD) $(LDFLAGS) -e main -Ttext 0x200000 -o ../../test/umain $(OBJECTS) $(LIBS)
	objdump -d ../../test/umain > ../../test/umain.s

clean:
	rm -rf $(OBJECTS) $(OBJECTS:.o=.d) ../../test/umain obj ../test

# Common rule
$(OBJ_DIR)/%.o: %.[cS]
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@
