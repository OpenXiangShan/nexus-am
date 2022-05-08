#include <test.h>

#define THREASHOLD 1
#define EXPECTED 100
#define MTIMEADDR 0x3800bff8

__attribute__((noinline))
void nemu_assert(int cond) {
  if (!cond) _halt(1);
}

int main() {
    int mtime;
    uintptr_t mcycle;
    int res;

    mtime = *(volatile uint32_t *)MTIMEADDR;
    asm volatile("csrr %0, mcycle" : "=r"(mcycle));

    res = (mcycle / EXPECTED - mtime);

    printf("mcycle :%ld, mtime: %d res = %d (`res` should belong to [-1,1])\n", mcycle, mtime, res);
    nemu_assert(res >= (0 - THREASHOLD) && res <= (0 + THREASHOLD));

    printf("valid\n");
    return 0;
}