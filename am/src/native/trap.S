.global __am_ret_from_trap
.global __am_asm_trap
.global __am_syscall
.global __am_async_ex

__am_asm_trap:
  pushq %rdi # cause number
  jmp __am_async_ex

__am_syscall:
  pushq $0

__am_async_ex:
  # will be used as the argument of __am_irq_handle, save it manually
  pushq %rdi
  # getcontext() does not preserve %rflags, %rax, %r10, %r11, save them manually
  pushfq
  pushq %r11
  pushq %r10
  pushq %rax
  # must have sizeof(ucontext_t) < 1024
  subq $1024, %rsp

  movq %rsp, %rdi
  call __am_irq_handle

__am_ret_from_trap:

  addq $1024, %rsp
  popq %rax
  popq %r10
  popq %r11

  testq %rdi, %rdi
  jnz __am_ret_from_intr

  popfq
  popq %rdi
  lea 8(%rsp), %rsp # cause, do not use addq, since it will change rflags!
  retq

__am_ret_from_intr:
  popfq
  popq %rdi
  lea 8(%rsp), %rsp # cause, do not use addq, since it will change rflags!
  retq $128
