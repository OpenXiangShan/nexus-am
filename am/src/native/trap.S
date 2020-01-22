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
  # getcontext() does not preserve %rflags, %rax, %r10, %r11,
  # also calling __am_irq_handle() requires %rdi to pass the argument,
  # save these registers manually
  pushfq
  pushq %rdi
  pushq %r11
  pushq %r10
  pushq %rax

  # must have sizeof(ucontext_t) < 1024
  subq $1024, %rsp

  movq %rsp, %rdi
  call __am_irq_handle

__am_ret_from_trap:
  # rdi now stores the interrupt flag saved at the end of __am_irq_handle()
  testq %rdi, %rdi
  jz no_need

  # now we are going enable interrupt
  # first save the caller-save registers
  pushq %rcx
  pushq %rdx
  pushq %rsi
  pushq %r8
  pushq %r9

  # rdi == 1 here
  call _intr_write

  popq %r9
  popq %r8
  popq %rsi
  popq %rdx
  popq %rcx

no_need:
  popq %rax
  popq %r10
  popq %r11

  movq 16(%rsp), %rdi  # c->cause updated at the end of __am_irq_handle()
  testq %rdi, %rdi
  popq %rdi
  jnz __am_ret_from_intr

  popfq
  lea 8(%rsp), %rsp # cause, do not use addq, since it will change rflags!
  retq

__am_ret_from_intr:
  popfq
  lea 8(%rsp), %rsp # cause, do not use addq, since it will change rflags!
  retq $128
