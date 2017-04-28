/*
 * Static runtime library for a system software on AbstractMachine
 */

#ifndef __KLIB_H__
#define __KLIB_H__

#include <am.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

size_t strlen(const char* s);
char* strcpy(char* dst, const char* src);
void* memset(void* v, int c, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
int atoi(const char* nptr);
char* itoa(int value, char* string, int radix);
int printk(const char* fmt, ...);
int sprintk(char* out, const char* fmt, ...);

#define assert(cond) \
  do { \
    if (!(cond)) { \
      printk("Assertion fail at %s:%d\n", __FILE__, __LINE__); \
      _halt(1); \
    } \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif
