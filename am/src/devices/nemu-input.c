#include <am.h>
#include <amdev.h>

#define KEYDOWN_MASK 0x8000

#ifdef __ARCH_X86_NEMU
#include <x86.h>
#define KBD_ADDR 0x60
#endif

#ifdef __ARCH_MIPS32_NEMU
#include <mips32.h>
#define KBD_ADDR 0x4060
#endif

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;
      int k = inl(KBD_ADDR);
      kbd->keydown = (k & KEYDOWN_MASK ? 1 : 0);
      kbd->keycode = k & ~KEYDOWN_MASK;
      return sizeof(_DEV_INPUT_KBD_t);
    }
  }
  return 0;
}
