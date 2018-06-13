#include <am.h>
#include <klib.h>


#define KB 1024
#define MB 1024 * 1024

unsigned int g_seed;

void fast_srand(int seed) {
  g_seed = seed;
}

int fast_rand(void) {
  g_seed = (214013*g_seed+2531011);
  return g_seed;
}

#define _STR(x) #x
#define MFC0(dst, src, sel) \
  asm volatile("mfc0 %0, $"_STR(src)", %1\n\t":"=r"(dst):"i"(sel))
#define MTC0(dst, src, sel) \
  asm volatile("mtc0 %0, $"_STR(dst)", %1\n\t"::"r"(src),"i"(sel))

#define NOP() \
  asm volatile("nop")

#define cp0_count  9

uint32_t inline GetCount(int sel){
  uint32_t tick = 0;
  if(sel == 0)
    MFC0(tick, cp0_count, 0);
  else if(sel == 1)
    MFC0(tick, cp0_count, 1);
  else
    _halt(1);
  return tick;
}

int main() {
  fast_srand(0x12345678);
  unsigned int steps = 1024;
  static char arr[4 * 1024 * 1024];
  int lengthMod;
  int sizes[] = { 10 * KB };
  int s;

  for (int j = 0; j < 10; j++) {
    for (s = 0; s < sizeof(sizes)/sizeof(int); s++) {
      lengthMod = sizes[s] - 1;
      for (unsigned int i = 0; i < steps; i++) {
        arr[fast_rand() & lengthMod] += 10;
      }
    }
  }
  return 0;
}
