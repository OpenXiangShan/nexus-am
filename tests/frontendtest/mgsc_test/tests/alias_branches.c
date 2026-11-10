#include <klib.h>
static volatile int sink;

#define BRANCH_SITE(i, pattern_len)                   \
    do {                                              \
        int bit = ((loop & ((pattern_len) - 1)) == 0);/*稀疏取反*/ \
        if (bit) sink += (i); else sink -= (i);       \
    } while (0)

int main(void) {
    for (int loop = 0; loop < 8000; loop++) {
        // 16 个“静态”分支位置（展开以制造邻近 PC，编译器通常会顺序放置）
        BRANCH_SITE(1, 2);
        BRANCH_SITE(2, 3);
        BRANCH_SITE(3, 4);
        BRANCH_SITE(4, 5);
        BRANCH_SITE(5, 6);
        BRANCH_SITE(6, 7);
        BRANCH_SITE(7, 8);
        BRANCH_SITE(8, 9);
        BRANCH_SITE(9, 10);
        BRANCH_SITE(10, 11);
        BRANCH_SITE(11, 12);
        BRANCH_SITE(12, 13);
        BRANCH_SITE(13, 14);
        BRANCH_SITE(14, 15);
        BRANCH_SITE(15, 16);
        BRANCH_SITE(16, 17);
    }
    return sink;
}
