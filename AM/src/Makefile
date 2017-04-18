ARCH = x86
CC = gcc
LD = ld
CFLAGS = -m32 -march=i386 -static -MD -std=gnu89 -ggdb \
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
		 -Wall -Werror -O2 -I../include
ASFLAGS = -ggdb -m32 -MD
LDFLAGS = -melf_i386

OBJ_DIR = obj
CSRC = $(shell find -L . -name "*.c")
SSRC = $(shell find -L . -name "*.S")
LIBS = $(shell find ../Lib/obj -name "*.o")
OBJECTS = $(SSRC:%.S=$(OBJ_DIR)/%.o) $(CSRC:%.c=$(OBJ_DIR)/%.o)
DOT_D = $(SSRC:.S=.d) $(CSRC:.c=.d)
ASMS = $(CSRC:.c=.S)

umain: $(OBJECTS)
	$(LD) $(LDFLAGS) -e main -Ttext 0x200000 -o umain $(OBJECTS) $(LIBS)

clean:
	rm -rf $(OBJECTS) $(OBJECTS:.o=.d) umain obj

# Common rule
$(OBJ_DIR)/%.o: %.[cS]
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@
