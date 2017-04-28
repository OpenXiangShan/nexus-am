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

// string.h
void* memset(void* v, int c, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
void* memmove(void* dst, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char* s);
char* strcat(char* dest, const char* src);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

// stdlib.h
int atoi(const char* nptr);
int abs(int x);

// stdio.h
int printk(const char* fmt, ...);
int sprintf(char* s, const char* format, ...);
int snprintf(char* s, size_t n, const char* format, ...);

// assert.h
#ifdef NDEBUG
  #define assert(ignore) ((void)0)
#else
  #define assert(cond) \
    do { \
      if (!(cond)) { \
        printk("Assertion fail at %s:%d\n", __FILE__, __LINE__); \
        _halt(1); \
      } \
    } while (0)

#endif

#ifdef __cplusplus
}
#endif

#endif
