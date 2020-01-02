.global __am_ret_from_trap
.global __am_asm_trap
.global __am_syscall
.global __am_async_ex

__am_asm_trap:
  pushq %rdi # irq number
  jmp __am_async_ex

__am_syscall:
  pushq $0

__am_async_ex:
  # will be used as the argument of __am_irq_handle, save it manually
  pushq %rdi
  pushq %rax
  # must have sizeof(ucontext_t) < 1024
  subq $1024, %rsp

  movq %rsp, %rdi
  call __am_irq_handle

__am_ret_from_trap:

  addq $1024, %rsp
  popq %rax
  popq %rdi
  addq $8, %rsp # irq
  retq
