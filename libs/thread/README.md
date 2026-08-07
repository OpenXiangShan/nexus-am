# AM Thread Library

[中文说明](README.zh-CN.md)

`libs/thread` is a small POSIX-like logical-thread library for the Abstract
Machine (AM).  It is designed for XiangShan multi-hart simulation, where each
hart executes at most one logical thread at a time.  The implementation lives
in `src/thread.c` and the public interface is `include/thread.h`.

## Capabilities

- Thread lifecycle: `thread_create`, `thread_join`, `thread_detach`,
  `thread_exit`, `thread_self`, and `thread_yield`.
- Synchronisation: normal, recursive, and error-checking mutexes; condition
  variables; barriers; and one-time initialization.
- Logical thread-specific data through `thread_key_*`, including destructor
  passes on thread exit.
- One shared FIFO ready queue for all configured harts.  A logical thread has
  no affinity and can resume on a different hart after yielding.
- Periodic timer preemption on `riscv64-xs-dual`.  Other targets are
  cooperative and require CPU-bound workers to call `thread_yield()`.

`thread_t` values start at 1.  ID 0 denotes the application main context.
Thread priority attributes are accepted for API compatibility but do not alter
FIFO scheduling.

## Initialization And Multi-Hart Bring-Up

No explicit thread-library initialization is required.  The first
`thread_create()` or `thread_yield()` calls `ensure_scheduler()` on CPU 0:

1. It installs the CTE yield handler and, on `riscv64-xs-dual`, the timer
   handler.
2. It publishes `thread_secondary_entry` through `_mpe_start()` and waits for
   every selected secondary hart to install its scheduler state.
3. It marks those secondary harts idle.  Newly created work is appended to the
   shared FIFO and an idle secondary is selected in round-robin order.

Before this first API call, the application must configure the hardware count:

```c
_mpe_setncpu('4');
```

The character must describe the actual XiangShan topology and be in the AM
range `1..MAX_CPU` (`MAX_CPU` is 8).  The linker reserves 128 KiB of stack/TLS
space for each hart.  This is an implementation and layout limit, not evidence
that eight-hart hardware has been validated.

`_mpe_wakeup(cpu)` has two distinct duties.  The first wake-up releases that
hart from reset by writing `HART_CTRL_RESET_REG_BASE + cpu * 8`; every wake-up
also writes `1` to `CLINT MSIP[cpu]` at `0x38000000 + cpu * 4`.  The latter is
required to notify a hart that is already running or parked.  Because the
current XiangShan emulator does not reliably resume an S-mode `wfi` after an
MSIP, the thread scheduler polls its ready queue while idle.  MSIP remains the
hardware notification path and reset release is performed only once.

## Scheduling And Context Switches

The scheduler protects a global FIFO ready queue with AM atomics.  A yield,
blocking synchronization operation, or timer tick saves the current logical
thread context, queues it at the tail when it remains runnable, and restores
the queue head on the hart that won the scheduler lock.  The active hart's
`tp` value is copied into a resumed context, so a thread can migrate without
using another hart's TLS base.

When `thread_create()` observes an idle secondary, it reserves that hart and
sets a short-lived `wake_pending` handoff.  Existing harts refrain from taking
the FIFO head until the selected hart consumes the handoff.  This prevents the
creator from immediately taking back a task intended for an idle hart.  After
the handoff, regular FIFO competition resumes; placement is deliberately not
guaranteed.

The main application context is represented as logical thread ID 0 only when
it first enters the scheduler.  Therefore applications can use the thread API
directly from `main()`.

## Timer Preemption On XiangShan

`riscv64-xs-dual` programs one `mtimecmp` and one `mscratch` timer state per
hart.  The library selects a 200 microsecond period before CTE changes
privilege level.  The interrupt path is:

```text
CLINT mtimecmp -> M-mode timer vector -> tagged SSIP -> CTE timer event
                -> thread_timer_handler -> shared FIFO scheduler
```

The M-mode vector advances that hart's compare register and raises SSIP.  CTE
recognizes its timer tag and dispatches the registered thread timer handler.
The scheduler ignores a tick that arrives while that hart owns the scheduler
lock, preventing recursive scheduling.  The next tick can schedule normally.

The library enables the required supervisor interrupt state and owns the AM
timer callback on this target.  It disables external interrupts.  Applications
that need their own timer or external-interrupt handler must integrate with the
CTE dispatcher rather than replacing the library handler.

For output from S-mode logical threads, use `s_atomic_printf`; it preserves
and restores `sstatus.SIE`.  `atomic_printf` retains the original M-mode
`mstatus` locking behavior for boot and M-mode callers.

## Basic Usage

```c
#include <am.h>
#include <thread.h>

static void *worker(void *arg) {
  return arg;
}

int main(void) {
  _mpe_setncpu('4');

  thread_t thread;
  void *result;
  thread_create(&thread, NULL, worker, (void *)42);
  thread_join(thread, &result);
  return result == (void *)42 ? 0 : 1;
}
```

Add `thread` to an AM application Makefile:

```make
LIBS += thread
```

`thread_attr_t` defaults to a 16 KiB joinable stack.  The minimum supported
stack is 4 KiB.  Thread control blocks and stacks come from the AM heap and
are intentionally not reclaimed, including detached threads.  This keeps the
bare-metal implementation simple but bounds the total number of threads by
heap capacity.

## Build And Test

From the repository root:

```sh
export AM_HOME="$PWD"
make -C libs/thread ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1
make -C apps/thread_smoke ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1 THREAD_SMOKE_NCPU=4
make -C apps/thread_demo ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1 THREAD_DEMO_NCPU=4
make -C tests/threadtest ARCH=riscv64-xs-dual LINUX_GNU_TOOLCHAIN=1
```

`apps/thread_smoke` is the regression test.  It checks lifecycle operations,
mutexes, barriers, condition variables, timeout handling, `thread_once`, TSD
destructors, all configured-hart worker coverage, and timer preemption.  Its
preemption phase runs one non-yielding worker more than the configured hart
count; it can complete only when a timer interrupt switches a busy worker.

`apps/thread_demo` is an interactive demonstration.  Set
`THREAD_DEMO_VERBOSE=0` to reduce UART traffic in long multi-hart runs.
`tests/threadtest` is an infinite printing stress program intended for visual
inspection of timer-driven switching, not a completion test.

The supplied differential reference model supports only one or two harts.  A
four-hart XiangShan run must disable differential testing, for example:

```sh
/path/to/emu -i apps/thread_smoke/build/thread_smoke-riscv64-xs-dual.bin \
  --no-diff
```

## Limits

- No priority scheduling, public CPU-affinity API, cancellation, or dynamic
  thread-stack/TCB reclamation.
- `thread_key_*` is logical-thread storage.  C `__thread` storage remains
  physical-hart local and must not be treated as logical TLS.
- Condition variables and barriers wait by yielding, not by maintaining a
  private blocking queue.
- On non-`riscv64-xs-dual` targets, CPU-bound routines must yield explicitly.
- Four-hart validation requires `--no-diff`; it is not supported by the
  current differential reference model.
