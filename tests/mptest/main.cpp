#include <am.h>
#include <amdev.h>
#include <klib.h>

void f() {
  // printf("%d / %d\n", _cpu(), _ncpu());
  while (1) {
    printf("%d", _cpu());
  }
}

int main() {
  _mpe_init(f);
  assert(0);
  return 0;
}
