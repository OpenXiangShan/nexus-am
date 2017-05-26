#include <am.h>
#include <npc.h>

extern char font8x8_basic[128][8];
extern char get_stat();
extern u32 GetCount(int sel);
u8 *fb;
int curr_line = 0;
int curr_col = 0;
ulong npc_time = 0;
ulong npc_cycles = 0;
static char *csend = SERIAL_PORT + Tx;
static char *crecv = SERIAL_PORT + Rx;

ulong _uptime(){
  ulong low = GetCount(0);
  ulong high = GetCount(1) + 1;
/*npc_time = high * 1000 * ((1ul << 32) / HZ) + low * 1000 / HZ;
 *npc_time = (high << 22) * 1000 * 1024 / HZ + low * 1000 / HZ;
*/
  npc_time = high * 1000 * ((1ul << 31) / HZ) * 2 + low / (HZ / 1000); //npc_time returns ms
  return npc_time;
}

ulong _cycles(){
  u32 low = GetCount(0);
  ulong high = GetCount(1) + 1;
/*npc_cycles = high * ((1ul << 32) >> 10) + (low >> 10);
 *npc_cycles = high * (1ul << 22) + (low >> 10);
 *npc_cycles = (high << 22) + (low >> 10);
*/
  npc_cycles = (high << 22) + (low >> 10); //npc_cycles returns Kcycles
  return npc_cycles;
}

void draw_character(char ch, int x, int y, _Pixel p) {
  int i, j;
  char *c = font8x8_basic[(int)ch];
  for (i = 0; i < 8; i ++) 
    for (j = 0; j < 8; j ++) 
      if ((c[i] >> j) & 1)
        _draw_p(y + j, x + i, p);
}

void draw_string(const char *str, int x, int y, _Pixel p) {
  while (*str) {
    draw_character(*str ++, x, y, p);
    if (y + 8 >= SCR_WIDTH) {
      x += 8; y = 0;
    } else {
      y += 8;
    }
  }
}

void vga_init(){
  _screen.width = SCR_WIDTH;
  _screen.height = SCR_HEIGHT;
  fb = VMEM_ADDR;
}

void out_byte(char ch) {
  while((get_stat() >> 3) & 0x1);
  *csend = ch;
}

char in_byte(){
  if(!(get_stat() & 0x1)) return '\0';
  else return *crecv;
}

void _putc(char ch) {
  //TODO:use uart
  //out_byte(ch);
  if(ch == '\n'){
    curr_col = 0;
    curr_line += 8;
  }
  else{
    draw_character(ch,curr_line,curr_col,pixel(0xff,0xff,0xff));
  }
  if (curr_col + 8 >= SCR_WIDTH) {
    curr_line += 8; curr_col = 0;
  } else {
    curr_col += 8;
  }
  if(curr_line >= SCR_HEIGHT){
    curr_line = 0;
  }
}

void _draw_f(_Pixel *p) {
  int i;
  for(i = 0;i < SCR_SIZE; i++){
    fb[i] = (R(p[i]) & 0xc0) | ((G(p[i]) & 0xf0) >> 2)| ((B(p[i]) & 0xc0) >> 6);
  }
}

void _draw_p(int x, int y, _Pixel p) {
  fb[x + y * _screen.width] = (R(p) & 0xc0) | ((G(p) & 0xf0) >> 2) | ((B(p) & 0xc0) >> 6);
}

void _draw_sync() {
}

int _read_key(){
  return 0;
}
