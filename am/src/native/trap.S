.global __am_ret_from_trap
.global __am_asm_trap

__am_asm_trap:
  # rip, rflags and interrupt flag are saved by the signal handler

  # getcontext() does not preserve %rax, %r10, %r11,
  # also calling __am_irq_handle() requires %rdi to pass the argument,
  # save these registers manually
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
  popq %rdi
  addq $8, %rsp
  popfq
  retq $128  # size of red zone
