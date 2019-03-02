#include <am.h>
#include <klib.h>

#ifndef __ARCH_RISCV32_NOOP
typedef union {
  struct { uint32_t lo, hi; };
  int64_t val;
} R64;
#endif

uint32_t softmul(uint32_t a, uint32_t b, int sign, int hi) {
  if (a == 0x80000000 && b == 0x80000000) {
    // hard code the result for this special case
    const R64 res = {.val = 0x4000000000000000LL};
    return (hi ? res.hi : res.lo);
  }
  else if (b == 0x80000000) {
    // swap the operands
    uint32_t t = a;
    a = b;
    b = t;
  }

  int sign_a = 0, sign_b = 0;
  if (sign) {
    if ((int32_t)a < 0) { sign_a = 1; a = -a; }
    if ((int32_t)b < 0) { sign_b = 1; b = -b; }
  }

  // booth algorithm
  R64 P = {.hi =  0, .lo = b};

  uint32_t choose = (P.lo & 1) << 1;
  int i;
  for (i = 32; i > 0; i --) {
    switch (choose) {
      case 1: P.hi += a; break;
      case 2: P.hi -= a; break;
    }
    choose = P.lo & 0x3;
    P.val >>= 1;
  }

  if (sign_a ^ sign_b) P.val = -P.val;
  return (hi ? P.hi : P.lo);
}

uint32_t softdiv(uint32_t a, uint32_t b, int sign, int reminder) {
  int sign_a = 0, sign_b = 0;
  if (sign) {
    if ((int32_t)a < 0) { sign_a = 1; a = -a; }
    if ((int32_t)b < 0) { sign_b = 1; b = -b; }
  }

  // restoring division

  R64 R = {.val = a};
  R.val <<= 1;
  int i;
  for (i = 32; i > 1; i --) {
    if (R.hi >= b) {
      R.hi -= b;
      R.val = (R.val << 1) + 1;
    }
    else {
      R.val <<= 1;
    }
  }

  R.lo <<= 1;
  if (R.hi >= b) {
    R.hi -= b;
    R.lo ++;
  }

  if (sign_a ^ sign_b) R.lo = -R.lo;
  if (sign_a) R.hi = -R.hi;
  return (reminder ? R.hi : R.lo);
}

void check_mul(int x, int y) {
  uint32_t lo = softmul(x, y, true, false);
  uint32_t hi = softmul(x, y, true, true);
  uint64_t res = ((uint64_t)hi << 32) | lo;

  uint64_t correct = (int64_t)x * y;

  if (res != correct) {
    printf("wrong: x = %d(0x%08x), y = %d(0x%08x)\n", x, x, y, y);
    printf("right = 0x%016llx\nwrong = 0x%016llx\n", correct, res);
  }
}

void check_div(int x, int y) {
  if (x == 0x80000000 && y == -1) return;
  uint32_t q = softdiv(x, y, true, false);
  uint32_t r = softdiv(x, y, true, true);

  uint32_t correct_q = x / y;
  uint32_t correct_r = x % y;

  if (q != correct_q || r != correct_r) {
    printf("x = %d(0x%08x), y = %d(0x%08x)\n", x, x, y, y);
    printf("right: q = 0x%08x, r = 0x%08x\nwrong: q = 0x%08x, r = 0x%08x\n",
        correct_q, correct_r, q, r);
  }
}

void check_divu(uint32_t x, uint32_t y) {
  uint32_t q = softdiv(x, y, false, false);
  uint32_t r = softdiv(x, y, false, true);

  uint32_t correct_q = x / y;
  uint32_t correct_r = x % y;

  if (q != correct_q || r != correct_r) {
    printf("x = %d(0x%08x), y = %d(0x%08x)\n", x, x, y, y);
    printf("right: q = 0x%08x, r = 0x%08x\nwrong: q = 0x%08x, r = 0x%08x\n",
        correct_q, correct_r, q, r);
  }
}

int v[] = {0, 1, 2, 3, 0x7fffffff, 0x80000000, 0x80000001, 0xfffffffd, 0xfffffffe, 0xffffffff};
#define NR_DATA(array) (sizeof(array) / sizeof(array[0]))

int main() {
  int i, j;
  for (i = 0; i < NR_DATA(v); i ++) {
    for (j = 1; j < NR_DATA(v); j ++) {
      check_mul(v[i], v[j]);
      check_div(v[i], v[j]);
      check_divu(v[i], v[j]);
    }
  }
  printf("PASS!\n");
  return 0;
}
