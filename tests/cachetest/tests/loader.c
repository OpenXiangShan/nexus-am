#include "klib.h"

int a[10] = {};

int f1(int n, int *a) {
  int sum = 0;
  for (int i = 0; i < n; i ++) {
    sum += a[i];
  }
  return sum;
}

int f2(int n, int *a) {
  int sum = 1;
  for (int i = 0; i < n; i ++) {
    sum *= a[i];
  }
  return sum;
}

uint32_t code[64];

int test(void *f) {
  printf("copy code...\n");
  memcpy(code, f, sizeof(code));

#ifdef __ISA_RISCV64__
  asm volatile("fence.i");
#endif

  int (*p)(int, int*) = (void *)code;
  printf("jump to  code...\n");
  int ret = p(10, a);
  printf("ret = %d\n", ret);
  return ret;
}

int main() {
#ifdef __ISA_NATIVE__
# error Can not run with native, since data segment is not executable.
#endif
  for (int i = 0; i < 10; i ++) {
    a[i] = i + 1;
  }

  int ret = test(f1);
  assert(ret == 55);

  ret = test(f2);
  assert(ret == 3628800);

  return 0;
}
