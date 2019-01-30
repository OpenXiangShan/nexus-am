start.o: start.S
	riscv-none-embed-gcc -I$(AM_HOME)/am/arch/riscv32-nemu/include -march=rv32im -mabi=ilp32 -fno-pic -MMD -c start.S -o start.o

-include start.d
