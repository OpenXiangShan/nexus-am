#ifndef __COUNTERTEST_H__
#define __COUNTERTEST_H__

#define MSG_ERROR   "\33[1;31mERROR\033[0m: "
#define MSG_WARNING "\33[1;33mWARNING\033[0m: "

#define MAP(c, f) c(f)

#define ACCESSIBLE 0
#define EX_II 2
#define EX_VI 22

extern int error;

#endif // __COUNTERTEST_H__
