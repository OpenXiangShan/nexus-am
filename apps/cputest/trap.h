#ifndef __TRAP_H__
#define __TRAP_H__

#define HIT_GOOD_TRAP \
  do { \
    _halt(0); \
  } while (0)

__attribute__((noinline))
void nemu_assert(int cond) {
  if (!cond) _halt(1);
}

#endif
