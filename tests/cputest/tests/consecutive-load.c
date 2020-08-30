#include "trap.h"

#define SIZE 4096
volatile unsigned char mem[SIZE] = { 0x0 };

int main() {
	unsigned i;

	int sum = 0;
	for(i = 0; i < SIZE; i ++) {
		sum += mem[i];
	}

	nemu_assert(sum == 0);

	return 0;
}
