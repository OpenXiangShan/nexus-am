#include <am.h>
#include <klib.h>

int main(){
  _ioe_init();
  int sec = 1;
  while (1) {
    while(_uptime() < 1000 * sec) ;
    printf("%d seconds.\n", sec ++);
  }
  return 0;
}
