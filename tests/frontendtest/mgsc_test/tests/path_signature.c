#include <klib.h>

static volatile int sink;
static volatile int always_true = 1;

__attribute__((noinline)) static void path_a(void) {
    // 分支 A：始终为 taken，留下的是 PC 信息而非方向差异
    if (always_true) sink++;
}

__attribute__((noinline)) static void path_b(void) {
    // 分支 B：同样始终 taken，方向历史保持一致
    if (always_true) sink += 2;
}

__attribute__((noinline)) static void shared_branch(int from_a) {
    // 共享分支：PC 相同，但方向仅由调用路径决定
    if (from_a) {
        sink += 7;
    } else {
        sink -= 5;
    }
}

int main(void) {
    unsigned lfsr = 1;
    void (*dispatch[2])(void) = {path_a, path_b};

    for (int i = 0; i < 6000; i++) {
        // 生成无分支的路径选择位
        lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
        int choose_a = lfsr & 1u;

        // 路径专属分支（方向相同），仅在路径历史中留痕
        dispatch[choose_a]();

        // 共享 PC 的真正测试分支
        shared_branch(choose_a);
    }
    return sink;
}
