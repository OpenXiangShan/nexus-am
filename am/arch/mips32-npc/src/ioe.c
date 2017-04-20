#include <am.h>
#include <npc.h>

void outb(char *port,char ch){
  *port = ch;
}

void _putc(char ch) {
  while(((*(SERIAL_PORT + STAT) >> 3) & 0x1));
  outb(SERIAL_PORT + Tx,ch);
}

void vga_init(){
  char *vmem = VMEM_ADDR;
  _screen.width = SCR_WIDTH;
  _screen.height = SCR_HEIGHT;
  int i;
  for(i = 0; i < SCR_SIZE; i++){
    vmem[i] = 0;
  }
}

void _draw_f(_Pixel *p) {
  char *vmem = VMEM_ADDR;
  int i;
  for(i = 0; i < SCR_SIZE; i++){
    vmem[i] = Getcolor(p[i]);
  }
}

void _draw_p(int x, int y, _Pixel p) {
  vmem[x * SCR_WIDTH + y] = Getcolor(p);
}

void _draw_sync() {
	//not to do
}

int peek_key(){
  return 0;
}

