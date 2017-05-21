
#include "trap.h"

int x = 0xffffffff;

int main() {

	asm volatile("lui $t0, 0x0102\n\t" \
			"ori $t0, $t0, 0x0304\n\t" \
			"sw $t0, 0x1000($zero)\n\t" \
			"lui $t0, 0x0506\n\t" \
			"ori $t0, $t0, 0x0708\n\t" \
			"sw $t0, 0x1004($zero)\n\t" \
			"lwl $t1, 0x1001($zero)\n\t" \
			"lwr $t1, 0x1006($zero)\n\t" \
			"sw $t1, %0\n\t" \
			:"=m"(x) \
	);

	nemu_assert(x == 0x03040506);

	HIT_GOOD_TRAP;

	return 0;
}
