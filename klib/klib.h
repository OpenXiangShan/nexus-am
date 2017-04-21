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

int strlen(const char *s);

#ifdef __cplusplus
}
#endif

#endif
