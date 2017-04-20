ARCH = mips
CROSS_COMPILE = mips-linux-gnu-
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -MD -fno-pic -static -fno-strict-aliasing -fno-builtin -EB
CFLAGS += -fno-stack-protector -mno-check-zero-division -fno-delayed-branch
CFLAGS += -Wall -Werror -march=mips32 -mno-llsc
CFLAGS += -mno-mad -std=gnu99
CFLAGS += -I../../../include/common -I../../../include/$(ARCH) -O2 -Wno-unused-function
CFLAGS += -mno-abicalls -g
CFLAGS += -D DEPLOY
SCFLAGS = -MD -fno-pic -static -fno-strict-aliasing -fno-builtin
SCFLAGS += -fno-stack-protector -mno-check-zero-division -fno-delayed-branch
SCFLAGS += -Wall -Werror -march=mips32 -mno-llsc -mno-imadd -mno-mad
SCFLAGS += -I../../../include/common -I../../../include/$(ARCH) -O2 -S -Wno-unused-function
ASFLAGS = -MD -mno-check-zero-division -mips32 -O0 -mno-abicalls
ASFLAGS += -fno-pic -fno-delayed-branch
LDFLAGS = 

OBJ_DIR = obj
CSRC = $(shell find  ../../test/ -name "*.c")
SSRC = $(shell find  ../../test/ -name "*.S")
LIBS = ../../../Lib/$(ARCH)/libam-$(ARCH).a
OBJECTS = $(SSRC:%.S=$(OBJ_DIR)/%.o) $(CSRC:%.c=$(OBJ_DIR)/%.o)
DOT_D = $(SSRC:.S=.d) $(CSRC:.c=.d)
ASMS = $(CSRC:.c=.S)

umain: ../../../build/$(ARCH)/*.o $(OBJECTS)
	$(LD) $(LDFLAGS) -T loader.ld -o ../../test/umain ../../../build/$(ARCH)/boot/start.o ../../../build/$(ARCH)/boot/bootmain.o ../../../build/$(ARCH)/*.o $(OBJECTS) $(LIBS)

clean:
	rm -rf $(OBJECTS) $(OBJECTS:.o=.d) ../../test/umain obj ../test

# Common rule
$(OBJ_DIR)/%.o: %.[cS]
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@
