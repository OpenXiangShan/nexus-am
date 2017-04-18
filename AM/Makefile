os.img:
	@cd arch/x86; make
	@cd Lib; make
	@cd src; make
	cat build/x86/x86-arch src/umain > os.img

play:
	qemu-system-i386 -serial stdio os.img

clean:
	@cd arch/x86; make clean
	@cd Lib; make clean
	@cd src; make clean
	-rm -f os.img
