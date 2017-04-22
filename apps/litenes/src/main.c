#include <am.h>
#include <fce.h>

extern char mario_nes[];

int main() {
  _trm_init();
  _ioe_init();

  fce_load_rom(mario_nes);
  fce_init();
  fce_run();
  return 1;
}
