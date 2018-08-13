#include <x86.h>

trap:
  cli
  pushl %ds
  pushl %es
  pushal

  movw $KSEL(SEG_KDATA), %ax
  movw %ax, %ds
  movw %ax, %es

  pushl %esp
  call irq_handle
  addl $4, %esp

  popal
  popl %es
  popl %ds
  addl $8, %esp

  iret

#define NOERR     pushl $0
#define ERR
#define IRQ_DEF(id, dpl, err) \
  .globl irq##id; irq##id: err; pushl $id; jmp trap;

IRQS(IRQ_DEF)
.globl irqall; irqall: pushl $0; pushl $-1; jmp trap
