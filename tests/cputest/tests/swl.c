
#include "trap.h"

unsigned x = 0xffffffff;

int main() {

	*((unsigned*)0x1003) = 0xaabbccdd;

	x = *((unsigned*)0x1003);
	nemu_assert(x == 0xaabbccdd);

	return 0;
}
