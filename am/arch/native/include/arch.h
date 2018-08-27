#ifndef __ARCH_H__
#define __ARCH_H__

#include <unistd.h>
#include <sys/types.h>

#define false 0
#define true 1

#ifndef __USE_GNU
# define __USE_GNU
#endif

#include <ucontext.h>

struct _Context {
  union {
    uint8_t pad[1024];
    struct {
      ucontext_t uc;
      void *prot;
    };
  };
  uintptr_t rax;
  uintptr_t rip;
};

#define GPR1 rax
#define GPR2 uc.uc_mcontext.gregs[REG_RSI]
#define GPR3 uc.uc_mcontext.gregs[REG_RDX]
#define GPR4 uc.uc_mcontext.gregs[REG_RCX]
#define GPRx rax

#define PGSHIFT 12
#define PGSIZE (1 << PGSHIFT)

#undef __USE_GNU

#endif
