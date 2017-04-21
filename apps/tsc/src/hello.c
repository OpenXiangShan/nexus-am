#include <hello.h>
#include <am.h>

int main() {
  _trm_init();
  _ioe_init();

  ulong t = _uptime();

  for (int i = 0; i <= 10000; i ++) {
    printi(_cycles());
    print("\n");
  }

  printi(_uptime() - t);
  print("ms\n");
  _halt(0);
}
