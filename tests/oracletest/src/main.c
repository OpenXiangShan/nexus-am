#include <test.h>

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
# define RTC_ADDR     0x3800bff8
#endif

#define TESTDUP 100
#define FINALITERATION 8000

volatile int counter;

void loop(int n) {
    for (volatile int i = 0; i < n; i++) {
        counter += 1;
    }
}

uint64_t get_time() {
    return *(uint64_t *)(RTC_ADDR);
}

int main() {
    printf("Oracle BP test shall always pass\n");
    printf("Check perf counter for OracleBP correctness\n");
    
    for (int i = 0; i < TESTDUP; i++) {
        int a = get_time() % 10;
        switch (a)
        {
        case 0:
            loop(5);
            break;
        case 1:
            loop(10);
            break;
        case 2:
            loop(15);
            break;
        case 3:
            loop(20);
            break;
        case 4:
            loop(25);
            break;
        case 5:
            loop(30);
            break;
        case 6:
            loop(35);
            break;
        case 7:
            loop(40);
            break;
        case 8:
            loop(45);
            break;
        case 9:
            loop(50);
            break;
        default:
            printf("Should not reach here! Check your test environment\n");
            break;
        }
    }

    for (volatile int i = 0; i < FINALITERATION; i++) {
        counter += i;
    }

    return 0;
}
