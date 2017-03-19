#define SERIAL_PORT 0x3f8

static void serial_init(){
	outb(SERIAL_PORT + 1,0x00);
	outb(SERIAL_PORT + 3,0x80);
	outb(SERIAL_PORT + 0,0x01);
	outb(SERIAL_PORT + 1,0x00);
	outb(SERIAL_PORT + 3,0x03);
	outb(SERIAL_PORT + 2,0xC7);
	outb(SERIAL_PORT + 4,0x0B);
}
