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
OBJECTS = $(SSRC:%.S=$(OBJ_DIR)/%.o) $(CSRC:%.c=$(OBJ_DIR)/%.o)
ASMS = $(CSRC:.c=.S)

# ********************
# Rules of Compilation
# ********************

all: $(OBJECTS)

# Common rule
$(OBJ_DIR)/%.o: %.[cS]
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -rf lib.a $(OBJECTS) obj *.txt> /dev/null
