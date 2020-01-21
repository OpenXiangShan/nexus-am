#include <sys/time.h>
#include "platform.h"

#define TIMER_HZ 100
enum { CAUSE_SYSCALL, CAUSE_YIELD, CAUSE_TIMER, CAUSE_IODEV };

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_asm_trap(int cause);
void __am_syscall();
void __am_async_ex();
void __am_ret_from_trap();

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

void __am_irq_handle(_Context *c) {
  getcontext(&c->uc);
  _intr_write(0);
  __am_get_cur_as(c);

  _Event e;
  int cause = c->cause;

  switch (cause) {
    case CAUSE_SYSCALL: e.event = _EVENT_SYSCALL; break;
    case CAUSE_YIELD  : e.event = _EVENT_YIELD; break;
    case CAUSE_TIMER  : e.event = _EVENT_IRQ_TIMER; break;
    case CAUSE_IODEV  : e.event = _EVENT_IRQ_IODEV; break;
    default: printf("Unhandle cause = %d\n", cause); assert(0);
  }
  _Context *ret = user_handler(e, c);
  if (ret != NULL) {
    c = ret;
  }

  __am_switch(c);
  c->uc.uc_mcontext.gregs[REG_RIP] = (uintptr_t)__am_ret_from_trap;
  // indicate different returning code, see trap.S
  c->uc.uc_mcontext.gregs[REG_RDI] = (c->cause == CAUSE_TIMER || c->cause == CAUSE_IODEV);
  c->uc.uc_mcontext.gregs[REG_RSP] = (uintptr_t)c;

  setcontext(&c->uc);
}

static void setup_stack(uintptr_t cause, ucontext_t *c) {
  uintptr_t *rip = (uintptr_t *)c->uc_mcontext.gregs[REG_RIP];
  extern uintptr_t _start, _etext;
  // assume the virtual address space of user process is not above 0xffffffff
  if (!((rip >= &_start && rip < &_etext) || (uintptr_t)rip < 0x100000000ul)) {
    // Shared libraries contain code which are not reenterable.
    // If the signal comes when executing code in shared libraries,
    // the signal handler can not call any function which is not signal-safe,
    // else the behavior is undefined (may be dead lock).
    // To handle this, we just refuse to handle the signal and return directly
    // to pretend missing the interrupt.
    // See man 7 signal-safety for more information.
    return;
  }
  // setup the stack as if we had called __am_async_ex();
  // use `-128` to skip the red zone of the stack frame, see the amd64 ABI manual for details
  uintptr_t rsp = c->uc_mcontext.gregs[REG_RSP] - 128;
  rsp -= sizeof(uintptr_t);
  *(uintptr_t *)rsp = (uintptr_t)rip;
  rsp -= sizeof(uintptr_t);
  // we directly put the cause number on the stack to avoid the corruption of %rdi
  *(uintptr_t *)rsp = cause;

  c->uc_mcontext.gregs[REG_RSP] = rsp;
  c->uc_mcontext.gregs[REG_RIP] = (uintptr_t)__am_async_ex;
}

static void sig_handler(int sig, siginfo_t *info, void *ucontext) {
  uintptr_t cause = (sig == SIGUSR1 ? CAUSE_IODEV : CAUSE_TIMER);
  setup_stack(cause, ucontext);
}

void __am_init_irq() {
  _intr_write(0);

  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_sigaction = sig_handler;
  s.sa_flags = SA_SIGINFO | SA_RESTART;
  __am_get_intr_sigmask(&s.sa_mask);

  int ret = sigaction(SIGVTALRM, &s, NULL);
  assert(ret == 0);
  ret = sigaction(SIGUSR1, &s, NULL);
  assert(ret == 0);

  struct itimerval it = {};
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 1000000 / TIMER_HZ;
  it.it_interval = it.it_value;
  ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  assert(ret == 0);
}

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  assert(sizeof(ucontext_t) < 1024);  // if this fails, allocate larger space in trap.S for ucontext

  *(uintptr_t *)0x100000 = (uintptr_t)__am_syscall;

  user_handler = handler;

  __am_init_irq();
  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  // (rsp + 8) should be multiple of 16 when
  // control is transfered to the function entry point.
  // See amd64 ABI manual for more details
  stack.end = (void *)(((uintptr_t)stack.end & ~15ul) - 8);
  _Context *c = (_Context*)stack.end - 1;

  __am_get_example_uc(c);
  c->rip = (uintptr_t)entry;
  int ret2 = sigemptyset(&(c->uc.uc_sigmask)); // enable interrupt
  assert(ret2 == 0);
  c->rflags = 0;
  c->cause = CAUSE_YIELD;

  c->rdi = (uintptr_t)arg;

  return c;
}

void _yield() {
  __am_asm_trap(CAUSE_YIELD);
}

int _intr_read() {
  sigset_t set;
  int ret = sigprocmask(0, NULL, &set);
  assert(ret == 0);
  int cli = sigismember(&set, SIGVTALRM);
  return !cli;
}

void _intr_write(int enable) {
  sigset_t set;
  __am_get_intr_sigmask(&set);

  // NOTE: sigprocmask does not supported in multithreading
  int ret = sigprocmask(enable ? SIG_UNBLOCK : SIG_BLOCK, &set, NULL);
  assert(ret == 0);
}
