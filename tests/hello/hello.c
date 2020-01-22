#include <am.h>
#include <klib.h>

void putd(int n) {
  _putc("0123456789"[n]);
}

int main(const char *args) {
  printf("args = \"");
  while (*args) {
    _putc(*args++);
  }
  printf("\"\n");

  printf("sizeof(long) = ");
  putd(sizeof(long));
  printf("\n");
  while (1);
  return 0;
}
