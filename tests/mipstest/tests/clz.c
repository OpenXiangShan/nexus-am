#include "trap.h"

int clz(uint32_t x) {
  if(!x) return 32;
  static const char debruijn32[32] = {
	0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19,
	1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18
  };
  x |= x>>1;
  x |= x>>2;
  x |= x>>4;
  x |= x>>8;
  x |= x>>16;
  x++;
  return debruijn32[x*0x076be629>>27];
}

int main() {
#ifdef __ISA_MIPS32__
  {
	// special case, in == 0
	int in = 0, out = 0;
	asm volatile("clz %0, %1": "=r"(out) : "r"(in));
	nemu_assert(out == clz(in));
  }

  for(int i = 0; i < 32; i++) {
	int in = 1 << i;
	int out = 0;
	asm volatile("clz %0, %1": "=r"(out) : "r"(in));
	nemu_assert(out == clz(in));
  }
#endif
  return 0;
}
