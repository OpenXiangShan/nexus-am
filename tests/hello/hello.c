#include <am.h>
#include <klib.h>

void hello() {
  printf("CPU %d: hello!\n", _cpu());
  while (1);
}
int main(const char *args) {
  printf("CPU0: hello with args=\"%s\"\n", args);
  _mpe_init(hello);
  return 0;
}
