
#include "undefined_lib.h"

void *memalign(size_t alignment, size_t size) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return NULL;
    }
    size_t total_size = size + alignment + sizeof(void *);

    void *raw_memory = malloc(total_size);
    uintptr_t raw_address = (uintptr_t)raw_memory;
    uintptr_t aligned_address = (raw_address + sizeof(void *) + alignment - 1) & ~(alignment - 1);
    void **aligned_ptr = (void **)aligned_address;

    aligned_ptr[-1] = raw_memory;

    return (void *)aligned_ptr;
}


int gettimeofday(struct timeval * rtc, void * null) {
    rtc->tv_usec = uptime();
    return 0;
}


int fprintf(void *file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = printf(format, args);
    return result;
}


float fabsf(float x) {
    return x < 0 ? -x : x;
}


double factorial(int n) {
    double result = 1.0;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

double power(double x, int n) {
    double result = 1.0;
    for (int i = 0; i < n; i++) {
        result *= x;
    }
    return result;
}

double toRadians(double degrees) {
    return degrees * (PI / 180.0);
}

float sinf(float x) {
    x = toRadians(x);
    float result = 0.0;
    int terms = 10;
    for (int n = 0; n < terms; n++) {
        int sign = (n % 2 == 0) ? 1 : -1;
        result += sign * (power(x, 2 * n + 1) / factorial(2 * n + 1));
    }
    return result;
}
float cosf(float x) {
    x = toRadians(x);
    float result = 0.0;
    int terms = 10;
    for (int n = 0; n < terms; n++) {
        int sign = (n % 2 == 0) ? 1 : -1;
        result += sign * (power(x, 2 * n) / factorial(2 * n));
    }
    return result;
}
