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
  pushl %eax
  pushl $0  // uvm

  movw $KSEL(SEG_KDATA), %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss

  pushl %esp
  call am_on_irq

.globl __am_iret
__am_iret:
  addl $4, %esp
  popl %eax
  movl %eax, %esp

  addl $4, %esp
  popl %eax
  movw %ax, %ds
  movw %ax, %es

  cmpw $KSEL(SEG_KDATA), %ax
  je .kernel_iret

  /* stack frame after popping %eax, %ebx, %ecx, %edx:
     28  ss
     24  esp  (not popped by iret when returning to ring0)
     20  eflags
     16  cs
     12  eip
      8  ebp 
      4  edi 
      0  esi <--- %esp
      */
 
.user_iret:
  popl %eax
  popl %ebx
  popl %ecx
  popl %edx
  popl %esi
  popl %edi
  popl %ebp
  iret
 
.kernel_iret:
  popl %eax
  popl %ebx
  popl %ecx
  popl %edx
 
  movl %esp, %ebp
  movl 24(%ebp), %edi   // %edi is new-esp

  movl 20(%ebp), %esi;  movl %esi, -4(%edi)
  movl 16(%ebp), %esi;  movl %esi, -8(%edi)
  movl 12(%ebp), %esi;  movl %esi, -12(%edi)
  movl  8(%ebp), %esi;  movl %esi, -16(%edi)
  movl  4(%ebp), %esi;  movl %esi, -20(%edi)
  movl  0(%ebp), %esi;  movl %esi, -24(%edi)

  leal -24(%edi), %esp

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
