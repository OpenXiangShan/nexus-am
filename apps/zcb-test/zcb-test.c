#include <klib.h>

#define C_LBU(rd, imm, rs1) asm volatile("c.lbu %0, %1(%2)" : "=r"(rd) : "i"(imm), "r"(rs1) : "memory")
#define C_LHU(rd, imm, rs1) asm volatile("c.lhu %0, %1(%2)" : "=r"(rd) : "i"(imm), "r"(rs1) : "memory")
#define C_LH(rd, imm, rs1)  asm volatile("c.lh %0, %1(%2)"  : "=r"(rd) : "i"(imm), "r"(rs1) : "memory")
#define C_SB(rs2, imm, rs1) asm volatile("c.sb %0, %1(%2)" : : "r"(rs2), "i"(imm), "r"(rs1) : "memory")
#define C_SH(rs2, imm, rs1) asm volatile("c.sh %0, %1(%2)" : : "r"(rs2), "i"(imm), "r"(rs1) : "memory")
#define C_ZEXT_B(x) asm volatile("c.zext.b %0"  : "+r"(x))
#define C_SEXT_B(x) asm volatile("c.sext.b %0"  : "+r"(x))
#define C_ZEXT_H(x) asm volatile("c.zext.h %0"  : "+r"(x))
#define C_SEXT_H(x) asm volatile("c.sext.h %0"  : "+r"(x))
#define C_ZEXT_W(x) asm volatile("c.zext.w %0"  : "+r"(x))
#define C_NOT(x)    asm volatile("c.not %0"     : "+r"(x))
#define C_MUL(x, y) asm volatile("c.mul %0, %1" : "+r"(x) : "r"(y))

void test_clbu() {
    uint64_t src = rand();
    uint64_t dest = 0;
    uint64_t ref = src & 0xff;
    C_LBU(dest, 0, &src);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xff00) >> 8;
    C_LBU(dest, 1, &src);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xff0000) >> 16;
    C_LBU(dest, 2, &src);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xff000000) >> 24;
    C_LBU(dest, 3, &src);
    assert(dest == ref);
}

void test_clhu() {
    uint64_t src = rand();
    uint64_t dest = 0;
    uint64_t ref = src & 0xffff;
    C_LHU(dest, 0, &src);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xffff0000) >> 16;
    C_LHU(dest, 2, &src);
    assert(dest == ref);
}

void test_clh() {
    uint64_t src = rand();
    uint64_t dest = 0;
    uint64_t ref = (int16_t)(src & 0xffff);
    C_LH(dest, 0, &src);
    assert(dest == ref);

    dest = 0;
    ref = (int16_t)((src & 0xffff0000) >> 16);
    C_LH(dest, 2, &src);
    assert(dest == ref);
}

void test_csb() {
    uint64_t src = rand();
    uint64_t dest = 0;
    uint64_t ref = src & 0xff;
    C_SB(src, 0, &dest);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xff) << 8;
    C_SB(src, 1, &dest);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xff) << 16;
    C_SB(src, 2, &dest);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xff) << 24;
    C_SB(src, 3, &dest);
    assert(dest == ref);
}

void test_csh() {
    uint64_t src = rand();
    uint64_t dest = 0;
    uint64_t ref = src & 0xffff;
    C_SH(src, 0, &dest);
    assert(dest == ref);

    dest = 0;
    ref = (src & 0xffff) << 16;
    C_SH(src, 2, &dest);
    assert(dest == ref);
}

void test_czext(int byte) {
    uint64_t x = rand();
    uint64_t ref = 0;
    switch (byte) {
        case 1: // zextb
            ref = x & 0xff;
            C_ZEXT_B(x);
            assert(x == ref);
            break;
        case 2: // zexth
            ref = x & 0xffff;
            C_ZEXT_H(x);
            assert(x == ref);
            break;
        case 4: // zextw
            ref = x & 0xffffffff;
            C_ZEXT_W(x);
            assert(x == ref);
            break;
        default:
            printf("invalid input\n");
            break;
    }
}

void test_csext(int byte) {
    uint64_t x = rand();
    int64_t ref = 0;
    switch (byte) {
        case 1: // sextb
            ref = (int8_t)(x & 0xff);
            C_SEXT_B(x);
            assert(x == ref);
            break;
        case 2: // sexth
            ref = (int16_t)(x & 0xffff);
            C_SEXT_H(x);
            assert(x == ref);
            break;
        default:
            printf("invalid input\n");
            break;
    }
}

void test_cnot() {
    int64_t x = rand();
    int64_t ref = ~x;
    C_NOT(x);
    assert(x == ref);
}

void test_cmul() {
    int64_t x = rand();
    int64_t y = rand();
    int64_t ref = x * y;
    C_MUL(x, y);
    assert(x == ref);
}

int main() {
    int n = 100;
    for (int i = 0; i < n; i++) {
        test_clbu();
        test_clhu();
        test_clh();

        test_csb();
        test_csh();

        test_czext(1); // b
        test_czext(2); // h
        test_czext(4); // w

        test_csext(1); // b
        test_csext(2); // h

        test_cnot();
        test_cmul();
    }
    printf("test pass\n");
    return 0;
}
