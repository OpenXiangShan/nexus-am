#ifndef __ARCH_H__
#define __ARCH_H__


struct _Context {
  struct _AddressSpace *uvm;
  uint64_t rax, rbx, rcx, rdx,
           rsi, rdi, rbp, rsp3,
           rip, eflags,
           cs, ds, es, ss, fs, gs,
           ss0, rsp0,
           r8, r9, r10, r11, r12, r13, r14, r15;
};


#define GPR1 rdi
#define GPR2 rsi
#define GPR3 rdx
#define GPR4 rcx
#define GPRx rax

#endif