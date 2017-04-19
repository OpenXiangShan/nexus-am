#ifndef NPC_ASSERT_H
#define NPC_ASSERT_H

extern void _bad(void);

#define _S(x) #x
#define S(x) _S(x)
#define assert(x) \
    do { \
        if (!(x)) { puts(__FILE__ "@" S(__LINE__) ":" "Assertion failed: " S(x)); _bad(); } \
    } while (0)

#endif
