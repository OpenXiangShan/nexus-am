#ifndef TRAP_H
#define TRAP_H

// NOTE: only for self-correctness check!

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(x) do { \
    if (!(x)) bad();   \
} while (0)


static void good() { puts("HIT GOOD TRAP"); exit(0); }
static void bad()  { puts("HIT BAD TRAP"); exit(-1); }

#endif // TRAP_H
