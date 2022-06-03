#include "klib.h"

extern uint8_t ram_start;

int main() {
  void (*f)(void) = (void *)&ram_start;
  f();
  return 0;
}
