#include "klib.h"

extern uint8_t image, image_end, ram_start;

void test(uint8_t *image_start, uint8_t *image_end) {
  printf("copy ram_start...\n");
  uint8_t *copy_dest = &ram_start;
  memcpy(copy_dest, image_start, image_end - image_start);
  printf("copy finish...\n");

#ifdef __ISA_RISCV64__
  asm volatile("fence.i");
#endif

  void (*f)(void) = (void *)copy_dest;
  printf("jump to ram_start...\n");
  f();
  //printf("ret = %d\n", ret);
}

int main() {
#ifdef __ISA_NATIVE__
# error Can not run with native, since data segment is not executable.
#endif
  test(&image, &image_end);

  return 0;
}
