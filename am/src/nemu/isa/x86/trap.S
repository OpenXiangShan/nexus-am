#----|------------entry------------|---usp---|---irq id---|-----handler-----|
.globl __am_vecsys;    __am_vecsys: pushl $0; pushl $0x80; jmp __am_asm_trap
.globl __am_vectrap;  __am_vectrap: pushl $0; pushl $0x81; jmp __am_asm_trap
.globl __am_irq0;        __am_irq0: pushl $0; pushl   $32; jmp __am_asm_trap
.globl __am_vecnull;  __am_vecnull: pushl $0; pushl   $-1; jmp __am_asm_trap

.globl __am_kcontext_start
__am_kcontext_start:
  pushl %eax
  call *%ebx

__am_asm_trap:
  cmpl $0, __am_ksp
  je in_kernel

  # switch to kernel stack
  xchg %esp, __am_ksp
  subl $20, %esp

in_kernel:
  pushal

  pushl $0

  pushl %esp
  call __am_irq_handle

  movl %eax, %esp

  addl $4, %esp
  popal
  addl $4, %esp

  iret   # customized with usp
