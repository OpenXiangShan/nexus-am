#define SCR_WIDTH 320
#define SCR_HEIGHT 200

void init_iom(){
    init_serial();
    init_vga();
    init_keyboard();
}
