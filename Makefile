ARCH = x86-linux

$(shell mkdir -p build/)

# AM library
AM_PATH = ./am/arch/$(ARCH)
AM_LIB = ./build/libam-$(ARCH).a
AM_SRC = $(shell find -L $(AM_PATH) -name "*.c" -o -name "*.cpp" -o -name "*.S")
AM_OBJ = $(addsuffix .o, $(basename $(AM_SRC)))

CFLAGS += -I ./am/
CXXFLAGS += -I ./am/ -I./am/arch/$(ARCH)/include -std=c++11

$(AM_LIB): $(AM_OBJ)
	ar rcs $(AM_LIB) $(AM_OBJ)

# TODO: merge code

mips:
	@cd src/makers/$(ARCH); make
	mips-linux-gnu-objcopy -O binary src/test/umain $(ARCH)_out/umain.bin
	mips-linux-gnu-objdump -d src/test/umain > $(ARCH)_out/code.txt
	python python/bin2text.py $(ARCH)_out/umain.bin $(ARCH)_out/ram.txt
	python python/gen_bram_coe.py $(ARCH)_out/umain.bin $(ARCH)_out/app.coe
	python python/instr_is_legal.py

clean:
	@cd arch/$(ARCH); make clean
	@cd Lib/$(ARCH); make clean
	@cd src/makers/$(ARCH); make clean
	-rm -rf $(ARCH)_out
