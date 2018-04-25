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

struct _RegSet {
  union {
    uint8_t pad[1024];
    ucontext_t uc;
  };
  uintptr_t rax;
  uintptr_t rip;
};

#define SYSCALL_ARG1(r) r->rax
#define SYSCALL_ARG2(r) r->uc.uc_mcontext.gregs[REG_RSI]
#define SYSCALL_ARG3(r) r->uc.uc_mcontext.gregs[REG_RDX]
#define SYSCALL_ARG4(r) r->uc.uc_mcontext.gregs[REG_RCX]

#undef __USE_GNU

#endif
