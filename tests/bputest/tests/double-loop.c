# define OutLoopNum 100
# define InnerLoopNum 50
# define RESULT OutLoopNum*inner
#include "trap.h"

int loop(int inner) {
    register int a=0, b=0;
    for (register int i = 0; i < OutLoopNum; i++) {
        for (register int j = 0; j < inner; j++) {
            b = a + 1;
            a = b + 1;
        }
    }
    return a / 2;
}

int tonum(const char s) {
    return (int)(s - '0');
}

int atoi(const char *s) {
    return tonum(s[0]) * 10 + tonum(s[1]);
}

int main (const char *args) {
    int inner = atoi(args);
    register int i = loop(inner);
    // printf("inner loop num is %d\n", inner);
    nemu_assert(i == RESULT);
    return 0;
}
