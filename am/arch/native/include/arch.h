#ifndef __ARCH_H__
#define __ARCH_H__

#include <unistd.h>
#include <sys/types.h>

#ifndef __USE_GNU
# define __USE_GNU
#endif

#include <ucontext.h>

struct _RegSet {
  gregset_t regs;
  // FIXME: should we save floating registers too?
};

#define SYSCALL_ARG1(r) r->regs[REG_RAX]
#define SYSCALL_ARG2(r) r->regs[REG_RSI]
#define SYSCALL_ARG3(r) r->regs[REG_RDX]
#define SYSCALL_ARG4(r) r->regs[REG_RCX]

#undef __USE_GNU

#endif
