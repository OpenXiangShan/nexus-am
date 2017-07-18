#include <am.h>

void print(const char *s) {
  for (; *s; s ++) {
    _putc(*s);
  }
}
int main() {
  asm volatile("li $1, 0x211");
  asm volatile("jr $1");
  for (int i = 0; i < 10; i ++) {
    print("Hello World!\n");
  }
  _halt(0);
}
/*int main(){
  _asye_init(NULL);
  while(1){
  }
}*/
