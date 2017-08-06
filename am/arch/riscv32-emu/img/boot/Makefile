start.o: start.S
	riscv32-unknown-elf-gcc -mabi=ilp32 -march=rv32i -I$(AM_HOME)/am/arch/riscv32-emu/include -fno-pic -ffunction-sections -MMD -c start.S -o start.o

-include start.d
