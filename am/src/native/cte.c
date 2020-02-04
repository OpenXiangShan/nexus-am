#include <sys/time.h>
#include "platform.h"

#define TIMER_HZ 100

#define YIELD_INSTR "0xff,0x14,0x25,0x08,0x00,0x10,0x00" // callq *0x100008
#define YIELD_INSTR_LEN ((sizeof(YIELD_INSTR)) / 5)  // sizeof() counts the '\0' byte
#define SYSCALL_INSTR_LEN YIELD_INSTR_LEN

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_asm_trap();
void __am_ret_from_trap();

void __am_get_cur_as(_Context *c);
void __am_get_empty_as(_Context *c);
void __am_switch(_Context *c);
int __am_in_userspace(void *addr);

void __am_irq_handle(_Context *c) {
  getcontext(&c->uc);
  __am_get_cur_as(c);

  _Context *ret = user_handler(thiscpu->ev, c);
  if (ret == NULL) { ret = c; }

  __am_switch(ret);

  // the original context constructed on the stack
  c = (void *)ret->uc.uc_mcontext.gregs[REG_RDI];
  *c = *ret;
  // interrupt flag, see trap.S
  c->uc.uc_mcontext.gregs[REG_RDI] = c->sti;
  c->uc.uc_mcontext.gregs[REG_RIP] = (uintptr_t)__am_ret_from_trap;
  c->uc.uc_mcontext.gregs[REG_RSP] = (uintptr_t)c + 1024;

  setcontext(&c->uc);
}

static void setup_stack(uintptr_t event, ucontext_t *c) {
  uint8_t *rip = (uint8_t *)c->uc_mcontext.gregs[REG_RIP];
  extern uint8_t _start, _etext;
  void *sigprocmask_base = &sigprocmask;
  // assume the virtual address space of user process is not above 0xffffffff
  if (((event == _EVENT_IRQ_IODEV) || (event == _EVENT_IRQ_TIMER)) &&
      !((rip >= &_start && rip < &_etext) ||
        // Hack here: "+13" points to the instruction after syscall.
        // This is the instruction which will trigger the pending signal
        // if interrupt is enabled.
        (rip == sigprocmask_base + 13) ||
        (uintptr_t)rip < 0x100000000ul)) {
    // Shared libraries contain code which are not reenterable.
    // If the signal comes when executing code in shared libraries,
    // the signal handler can not call any function which is not signal-safe,
    // else the behavior is undefined (may be dead lock).
    // To handle this, we just refuse to handle the signal and return directly
    // to pretend missing the interrupt.
    // See man 7 signal-safety for more information.
    return;
  }

  // skip the instructions causing SIGSEGV for syscall and yield
  if (event == _EVENT_SYSCALL) { rip += SYSCALL_INSTR_LEN; }
  else if (event == _EVENT_YIELD) { rip += YIELD_INSTR_LEN; }

  // skip the red zone of the stack frame, see the amd64 ABI manual for details
  uintptr_t rsp = c->uc_mcontext.gregs[REG_RSP] - RED_NONE_SIZE;

#define PUSH(x) rsp -= sizeof(uintptr_t); *(uintptr_t *)rsp = (uintptr_t)(x)
  PUSH(rip);
  // rflags is not preserved by getcontext(), save it here
  PUSH(c->uc_mcontext.gregs[REG_EFL]);
  uintptr_t sti = __am_is_sigmask_sti(&c->uc_sigmask);
  PUSH(sti);

  // disable interrupt
  __am_get_intr_sigmask(&c->uc_sigmask);

  c->uc_mcontext.gregs[REG_RSP] = rsp;
  c->uc_mcontext.gregs[REG_RIP] = (uintptr_t)__am_asm_trap;
}

static void sig_handler(int sig, siginfo_t *info, void *ucontext) {
  thiscpu->ev = (_Event) {0};
  thiscpu->ev.event = _EVENT_ERROR;
  switch (sig) {
    case SIGUSR1: thiscpu->ev.event = _EVENT_IRQ_IODEV; break;
    case SIGVTALRM: thiscpu->ev.event = _EVENT_IRQ_TIMER; break;
    case SIGSEGV:
      if (info->si_code == SEGV_ACCERR) {
        switch ((uintptr_t)info->si_addr) {
          case 0x100000: thiscpu->ev.event = _EVENT_SYSCALL; break;
          case 0x100008: thiscpu->ev.event = _EVENT_YIELD; break;
        }
      }
      if (__am_in_userspace(info->si_addr)) {
        assert(thiscpu->ev.event == _EVENT_ERROR);
        thiscpu->ev.event = _EVENT_PAGEFAULT;
        switch (info->si_code) {
          case SEGV_MAPERR: thiscpu->ev.cause = _PROT_READ; break;
          // we do not support mapped user pages with _PROT_NONE
          case SEGV_ACCERR: thiscpu->ev.cause = _PROT_WRITE; break;
          default: assert(0);
        }
        thiscpu->ev.ref = (uintptr_t)info->si_addr;
      }
      break;
    default: assert(0);
  }
  assert(thiscpu->ev.event != _EVENT_ERROR);
  setup_stack(thiscpu->ev.event, ucontext);
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
  assert(SYSCALL_INSTR_LEN == 7);

  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_sigaction = sig_handler;
  s.sa_flags = SA_SIGINFO;
  __am_get_intr_sigmask(&s.sa_mask);
  int ret = sigaction(SIGSEGV, &s, NULL);
  assert(ret == 0);

  user_handler = handler;

  __am_init_irq();
  return 0;
}

void _kcontext(_Context *c, _Area stack, void (*entry)(void *), void *arg) {
  // (rsp + 8) should be multiple of 16 when
  // control is transfered to the function entry point.
  // See amd64 ABI manual for more details
  stack.end = (void *)(((uintptr_t)stack.end & ~15ul) - 8);
  stack.end -= RED_NONE_SIZE;
  _Context *c_on_stack = (_Context*)stack.end - 1;

  __am_get_example_uc(c);
  c->rip = (uintptr_t)entry;
  c->sti = 1;
  c->rflags = 0;
  __am_get_empty_as(c);

  c->rdi = (uintptr_t)arg;
  c->uc.uc_mcontext.gregs[REG_RDI] = (uintptr_t)c_on_stack; // used in __am_irq_handle()
}

void _yield() {
  asm volatile (".byte " YIELD_INSTR);
}

int _intr_read() {
  sigset_t set;
  int ret = sigprocmask(0, NULL, &set);
  assert(ret == 0);
  return __am_is_sigmask_sti(&set);
}

void _intr_write(int enable) {
  extern sigset_t __am_intr_sigmask;
  // NOTE: sigprocmask does not supported in multithreading
  int ret = sigprocmask(enable ? SIG_UNBLOCK : SIG_BLOCK, &__am_intr_sigmask, NULL);
  assert(ret == 0);
}
