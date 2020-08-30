#include "trap.h"

#define SIZE 4096
volatile int mem[SIZE] = { 0x0 };

int main() {
	int i;

	for(i = 0; i < SIZE; i ++) {
		mem[i] = i;
	}

	return 0;
}
