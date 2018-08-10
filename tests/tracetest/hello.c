#include <am.h>
#include <amtrace.h>

void print(const char *s) {
  for (; *s; s ++) {
    _putc(*s);
  }
}
int main() {
  _trace_on();
  _ioe_init();
  print("Hello World!\n");
  return 1;
}
