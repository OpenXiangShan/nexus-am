ARCH = x86

all:
	@cd arch/$(ARCH); make
	@cd Lib/$(ARCH); make
	@cd src/makers/$(ARCH); make
	-mkdir $(ARCH)_out

x86:
	cat build/$(ARCH)/$(ARCH)-arch src/test/umain > $(ARCH)_out/os.img
	qemu-system-i386 -serial stdio $(ARCH)_out/os.img

clean:
	@cd arch/$(ARCH); make clean
	@cd Lib/$(ARCH); make clean
	@cd src/makers/$(ARCH); make clean
	-rm -rf $(ARCH)_out
