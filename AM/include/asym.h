#ifndef __ASYM_H__
#define __ASYM_H__

#include <common.h>

__H_BEGIN__

void _asym_init();

// TODO: use a struct to describe an exception.

// claim a listener to all exceptions
//   call @l(ex, regs) at exception #ex, context @regs
//   returns the context to be switched (nullptr for do nothing)
void _listen(_RegSet* (*l)(int ex, _RegSet *regs));

// create a context:
//   stack pointer @stack
//   code entry @entry
//   @narg arguments are passed to the entry function
void _make(_RegSet *c, void *stack, void *entry, int nargs, ...);

void _trap(); // self-trap for context switch
void _idle(); // wait for next interrupt
void _ienable(); // enable interrupts
void _idisable(); // disable interrupts
int _istatus(); // interrupt enable = 1, else = 0

// atomic operations

__H_END__

#endif
