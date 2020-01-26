#include <x86.h>

trap:
  cli

  subl $20, %esp
  pushl %ebp
  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx
  pushl %ebx
  pushl %eax
  movw %ds, %ax
  pushl %ax
  pushl $0  // uvm

  movw $KSEL(SEG_KDATA), %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss

  pushl %esp
  call am_on_irq

__am_iret:
  movl 
  addl $4, %esp
  popl %eax
  movl %eax, %esp

  addl $4, %esp
  popl %eax
  movw %ax, %ds
  movw %ax, %es
  popl %eax
  popl %ebx
  popl %ecx
  popl %edx
  popl %esi
  popl %edi
  popl %ebp
  iret

#define NOERR     push $0
#define ERR
#define IRQ_DEF(id, dpl, err) \
  .globl __am_irq##id; __am_irq##id: cli;      err; push $id; jmp trap;
IRQS(IRQ_DEF)
  .globl  __am_irqall;  __am_irqall: cli; push $0; push $-1; jmp trap;
