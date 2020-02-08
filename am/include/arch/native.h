#ifndef __ARCH_H__
#define __ARCH_H__

#ifndef __USE_GNU
# define __USE_GNU
#endif

#include <ucontext.h>

struct _Context {
  union {
    uint8_t pad[1024];
    struct {
      ucontext_t uc;
      void *as;
    };
  };
  uintptr_t rax, r10, r11, rdi; // registers not preserved by getcontext()
  uintptr_t sti, rflags, rip;   // saved by signal handler
  uint8_t redzone[128];
};

#define GPR1 rdi
#define GPR2 uc.uc_mcontext.gregs[REG_RSI]
#define GPR3 uc.uc_mcontext.gregs[REG_RDX]
#define GPR4 uc.uc_mcontext.gregs[REG_RCX]
#define GPRx rax

#undef __USE_GNU

#endif
