ARCH = x86-linux

$(shell mkdir -p build/)

# AM library
AM_PATH = ./am/arch/$(ARCH)
AM_LIB = ./build/libam-$(ARCH).a
AM_SRC = $(shell find -L $(AM_PATH) -name "*.c" -o -name "*.cpp" -o -name "*.S")
AM_OBJ = $(addsuffix .o, $(basename $(AM_SRC)))

# TODO: managing flags
CFLAGS += -std=c99 -I ./am/ -I./$(AM_PATH)/include
CXXFLAGS += -std=c++11 -I ./am/ -I./$(AM_PATH)/include

all: $(AM_LIB) build/a
	@true

$(AM_LIB): $(AM_OBJ)
	ar rcs $(AM_LIB) $(AM_OBJ)

# These are temporary.
# TODO: generic application making
build/a:
	gcc $(CFLAGS) -c -o build/hello.o apps/hello/src/hello.c -I./apps/hello/include
	gcc $(CFLAGS) -c -o build/print.o apps/hello/src/print.c -I./apps/hello/include
	ar rcs ./build/hello.a build/hello.o build/print.o
	$(AM_PATH)/img/build ./build/hello.a
	@echo "====== execute ======"
	@./build/a.out



# TODO: merge code

mips:
	@cd src/makers/$(ARCH); make
	mips-linux-gnu-objcopy -O binary src/test/umain $(ARCH)_out/umain.bin
	mips-linux-gnu-objdump -d src/test/umain > $(ARCH)_out/code.txt
	python python/bin2text.py $(ARCH)_out/umain.bin $(ARCH)_out/ram.txt
	python python/gen_bram_coe.py $(ARCH)_out/umain.bin $(ARCH)_out/app.coe
	python python/instr_is_legal.py

clean:
	rm -rf build/ $(shell find . -name "*.o" -o -name "*.d")
