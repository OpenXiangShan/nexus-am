.global ret_from_trap
.global asm_trap

asm_trap:
  # getcontext() does not preserve %rax, save it manually
  pushq %rax
  # must have sizeof(ucontect_t) < 1024
  subq $1024, %rsp

  movq %rsp, %rdi
  call irq_handle

ret_from_trap:

  addq $1024, %rsp
  popq %rax
  retq
