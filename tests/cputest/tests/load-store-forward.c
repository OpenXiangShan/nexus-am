#include "trap.h"

#define SIZE 4096
volatile int mem[SIZE] = { 0x0 };

int main() {
	int i;

	for(i = 1; i < SIZE; i ++) {
		mem[i] = mem[i - 1] + 1;
	}

	nemu_assert(mem[SIZE - 1] == SIZE - 1);

	return 0;
}
