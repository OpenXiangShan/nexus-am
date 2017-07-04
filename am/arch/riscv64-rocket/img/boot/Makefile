start.o: start.S
	riscv64-unknown-linux-gnu-gcc -mabi=lp64 -march=rv64imac -I$(AM_HOME)/am/arch/riscv64-rocket/include -fno-pic -ffunction-sections -MMD -c start.S -o start.o

-include start.d
