start.o: start.S
	mips-linux-gnu-gcc -I$(AM_HOME)/am/arch/mips32-nemu/include -EL -march=mips32 -fno-pic -MMD -mno-abicalls -fno-delayed-branch -c start.S -o start.o

-include start.d
