#include <klib.h>

static volatile int sink = 0;
static inline int from_site0(void) { return 1; }
static inline int from_site1(void) { return -1; }

int main(void) {
    int last = 0;
    for (int i = 0; i < 4000; i++) {
        if (i & 1) {
            last = from_site0(); // call site A
        } else {
            last = from_site1(); // call site B
        }
        // 分支方向仅依赖“上一次调用来自哪个路径”
        if (last > 0) {
            sink += 3;
        } else {
            sink -= 2;
        }
    }
    return sink;
}
