#define SCR_WIDTH 320
#define SCR_HEIGHT 200
#include "vga.h"

void init_iom(){
    init_vga();
    init_keyboard();
}
