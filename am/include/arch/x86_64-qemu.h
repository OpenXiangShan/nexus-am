#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  struct _AddressSpace *uvm;
  uint64_t rax, rbx, rcx, rdx,
           rbp, rsi, rdi,
           r8, r9, r10, r11,
           r12, r13, r14, r15,
           rip, cs, rflags,
           rsp, ss, rsp0, ss0;
};


#define GPR1 rdi
#define GPR2 rsi
#define GPR3 rdx
#define GPR4 rcx
#define GPRx rax

#endif