#ifndef __TRM_H__
#define __TRM_H__

#include <common.h>

__H_BEGIN__

void _start(); // entry point to call main()
void _trm_init(); // initialize TRM

  // debugging
void _putc(char ch); // print a char to the debug console
void _panic(int code); // halt with error code, 0 = normal exit

// consecutive memory
typedef struct _Area {
  void *start, *end;
} _Area; 

extern _Area _heap;

__H_END__

#endif
