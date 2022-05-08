#include <amtest.h>

int main() {
    uintptr_t mcycle = -1;
    asm volatile("csrr %0, mcycle" : "=r"(mcycle));

    nemu_assert(mcycle / uptime() == 100);

    return 0;
}