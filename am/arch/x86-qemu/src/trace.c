#include <am-x86.h>

volatile int trace_on = 0;

void _trace_on() {
  _atomic_xchg(&trace_on, 1);
}

void _trace_off() {
  _atomic_xchg(&trace_on, 0);
}
