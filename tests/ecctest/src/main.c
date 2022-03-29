#include <test.h>

void case_passed() {
    static int i = 0;
    printf("Test case %d passed.\n", i++);
    __asm__("ret");
}

void success() {
    printf("Cache op test passed.\n");
    asm("li a0, 0\n");
    asm(".word 0x0000006b\n");
}

void failure() {
    printf("Cache op test failed\n");
    asm("li a0, 1\n");
    asm(".word 0x0000006b\n");
}

// volatile int victim = 0;
// volatile int * victim;


int main() {
    // printf("Cache op test: difftest should be disabled\n");
    // printf("Hint: use --no-diff to disable difftest\n");
    // victim = data_page_00;
    // victim = (int*)0x80002000;
    workload();

    // int * victim = (int *)0x80002000;
    // int tmp = *victim;

    return 1;
}
