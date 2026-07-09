#ifndef __APPS_MT_COMMON_H__
#define __APPS_MT_COMMON_H__

#include <am.h>
#include <xsextra.h>
#include <klib.h>

#define NR_HARTS 2

int read_hartid(void);
void fence_rw(void);
void cpu_relax(void);
void busy_delay(int cycles);
void spin_lock(volatile intptr_t *lock_word);
void spin_unlock(volatile intptr_t *lock_word);
void fail_now(const char *sample, int hartid, const char *reason) __attribute__((noreturn));
void expect(int cond, const char *sample, int hartid, const char *reason);
void finish_case(const char *sample, int hartid) __attribute__((noreturn));
void setup_dual_harts(int hartid);

#endif
