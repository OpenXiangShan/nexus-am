#include <test.h>

void case_passed() {
    static int i = 0;
    printf("Test case %d passed.\n", i++);
    __asm__("ret");
}

void success() {
    printf("Cache op test passed.\n");
    while(1);
}

void failure() {
    printf("Cache op test failed\n");
    while(1);
}

int main() {
    printf("Cache op test: difftest should be disabled\n");
    printf("Hint: use --no-diff to disable difftest\n");
    workload();
    return 0;
}
