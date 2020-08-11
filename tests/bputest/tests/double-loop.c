# define OutLoopNum 100
# define InnerLoopNum 2
# define RESULT OutLoopNum*InnerLoopNum*2
#include "trap.h"

int add() {
    register int a=0,b=0;
    for (register int i = 0; i < OutLoopNum; i++) {
        for (register int j = 0; j < InnerLoopNum; j++) {
            b++;
            a++;
        }
    }
    return a+b;
}

int main () {
    register int r = add();
    nemu_assert(r == RESULT);
    return 0;
}
