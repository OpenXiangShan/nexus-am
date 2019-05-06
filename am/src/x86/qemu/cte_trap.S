#include <x86.h>

trap:
  pushl %ds
  pushl %es
  pushal

  movw $KSEL(SEG_KDATA), %ax
  movw %ax, %ds
  movw %ax, %es

  pushl %esp
  call __am_irq_handle // iret in the C code

#define NOERR     pushl $0
#define ERR
#define IRQ_DEF(id, dpl, err) \
  .globl __am_irq##id; __am_irq##id: cli;      err; pushl $id; jmp trap;
IRQS(IRQ_DEF)
  .globl  __am_irqall;  __am_irqall: cli; pushl $0; pushl $-1; jmp trap;
