#ifndef __UNDEFINED_LIBC__
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/mman.h>
// #include <unistd.h>
// #include <stdint.h>
#include "klib.h"
#include "printf.h"
#define PI 3.14159265358979323846

struct timeval {
    long tv_sec;
    long tv_usec;
};

void *memalign(size_t alignment, size_t size);
int gettimeofday(struct timeval *rtc, void * null);
int fprintf(void *file, const char *format, ...);
float fabsf(float x);
float sinf(float x);
float cosf(float x);
#endif