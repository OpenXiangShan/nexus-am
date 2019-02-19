#----|-------entry-------|-errorcode-|---irq id---|---handler---|
.globl vecsys;    vecsys:  pushl $0;  pushl $0x80; jmp asm_trap
.globl vectrap;  vectrap:  pushl $0;  pushl $0x81; jmp asm_trap
.globl irq0;        irq0:  pushl $0;  pushl   $32; jmp asm_trap
.globl vecnull;  vecnull:  pushl $0;  pushl   $-1; jmp asm_trap

asm_trap:
  pushal

  pushl $0

  pushl %esp
  call irq_handle

  movl %eax, %esp

  addl $4, %esp
  popal
  addl $8, %esp

  iret
