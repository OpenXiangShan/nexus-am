#include <am.h>
#include <klib.h>


void foo() {
  while (1) _putc('.');
}


int main(const char *args) {
  printf("args = %s\n", args);
  return 0;
}
