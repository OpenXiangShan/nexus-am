$(info Building bootblock [$(ARCH)])

mbr: start.S main.c
	@echo + CC start.S main.c
	@gcc -nostdlib -Os start.S main.c -m32 -Ttext 0x7C00 -o bootblock.o
	@objcopy -S -O binary -j .text bootblock.o mbr
	@perl genboot.pl mbr

clean:
	rm mbr -rf bootblock *.o
