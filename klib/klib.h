/*
 * Static runtime library for a system software on AbstractMachine
 */

#ifndef __KLIB_H__
#define __KLIB_H__

#include <am.h>

#ifdef __cplusplus
extern "C" {
#endif

// We're expecting:
//   assert, printk, sprintk
//   memcpy, memset, strcpy, strlen, itoa, atoi, ...
//   kalloc, kree

size_t strlen(const char *s);
char* strcpy(char *dst,const char *src);
void* memset(void* v,int c,size_t n);
void* memcpy(void* dst,const void*src,size_t n);
int atoi(const char* nptr);
char* itoa(int value,char* string,int radix);

void assert (int expression);
int printk(const char *fmt, ...);
int sprintk(char *out, const char *fmt, ...);
int rand();

#ifdef __cplusplus
}
#endif

#endif
