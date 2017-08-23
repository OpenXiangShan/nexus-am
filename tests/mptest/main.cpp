#include <am.h>
#include <amdev.h>
#include <klib.h>

void f() {
  printf("%d / %d\n", _cpu(), _ncpu());
  while (1);
}

int main() {
  _asye_init(nullptr);
  _mpe_init(f);
  assert(0);
  return 0;
}
