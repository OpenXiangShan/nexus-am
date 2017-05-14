#include <am.h>
#include <npc.h>

int curr_line = 0;
int curr_col = 0;
extern char font8x8_basic[128][8];
u8 *fb;
extern char get_stat();
static char *csend = SERIAL_PORT + Tx;
static char *crecv = SERIAL_PORT + Rx;

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
  out_byte(ch);
  /*if(ch == '\n'){
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
}*/
}

void _draw_f(_Pixel *p) {
  int i;
  for(i = 0;i < SCR_SIZE; i++){
    fb[i] = R(p[i]) << 8 | G(p[i]) << 4 | B(p[i]);
  }
}

void _draw_p(int x, int y, _Pixel p) {
  fb[x + y * _screen.width] = R(p) << 8 | G(p) << 4 | B(p);
}

void _draw_sync() {
  //not to do
}

static inline int keydown(int e) { return (e & 0x8000) != 0; }
static inline int upevent(int e) { return e; }
static inline int downevent(int e) { return e | 0x8000; }

int _peek_key(){
  /*int key_code = in_byte();
  switch(key_code){
    case 'j': return downevent(_KEY_Z);break;
    case 'k': return downevent(_KEY_X);break;
    case 'w': return downevent(_KEY_UP);break;
    case 's': return downevent(_KEY_DOWN);break;
    case 'a': return downevent(_KEY_LEFT);break;
    case 'd': return downevent(_KEY_RIGHT);break;
    case 0xf0:{
        switch(in_byte()){
    	  case 'j': return upevent(_KEY_Z);break;
    	  case 'k': return upevent(_KEY_X);break;
    	  case 'w': return upevent(_KEY_UP);break;
    	  case 's': return upevent(_KEY_DOWN);break;
    	  case 'a': return upevent(_KEY_LEFT);break;
    	  case 'd': return upevent(_KEY_RIGHT);break;
          default: return upevent(_KEY_NONE);
        }
    }
    default: return upevent(_KEY_NONE);
  }*/
  return 0;
}
