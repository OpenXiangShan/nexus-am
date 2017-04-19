ARCH = x86

all:
	@cd arch/$(ARCH); make
	@cd Lib/$(ARCH); make
	-mkdir $(ARCH)_out

x86:
	@cd src/makers/$(ARCH); make
	cat build/$(ARCH)/$(ARCH)-arch src/test/umain > $(ARCH)_out/os.img
	qemu-system-i386 -serial stdio $(ARCH)_out/os.img

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
