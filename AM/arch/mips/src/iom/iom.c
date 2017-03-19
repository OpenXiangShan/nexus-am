#define SCR_WIDTH 320
#define SCR_HEIGHT 200
#include "vga.h"

surface screen;

void init_iom(){
    screen.width = SCR_WIDTH;
    screen.height = SCR_HEIGHT;
    pixel p_screen[SCR_HEIGHT * SCR_WIDTH];
    screen.pixels = p_screen;
}
