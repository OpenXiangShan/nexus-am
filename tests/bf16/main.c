#include "bf16.h"
#include <klib.h>
#include <stdint.h>



int main() {
  printf("=== BF16 Function Tests ===\n\n");
  asm volatile("fsrm zero" : : : "memory");
  INIT();
  
  test_fmv();
  test_store_load_half();
  test_float_to_bf16();
  test_vfncvtbf16();
  test_vfwcvtbf16();
  test_vfwmaccbf16();
  test_vfwmaccbf16_vf();

  printf("=== All Tests Completed ===\n");
  return 0;
}
