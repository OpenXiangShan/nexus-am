#include <am.h>
#include <klib.h>
//_uptime() to get now time

int main(){
  _ioe_init();
  //_asye_init();
  while(_uptime() < 10000);
  char *str = "Pass";
  while(*str){_putc(*str); str++;}
  _halt(0);
  return 0;
}
