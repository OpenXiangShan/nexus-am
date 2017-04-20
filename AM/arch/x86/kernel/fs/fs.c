#include "x86.h"
#include "device.h"

union character {
	uint16_t val;
	struct {
	uint8_t ch;
	uint8_t color;
	};
};

int write(int fd, char *buf, int len, int line, int row) {
	if(fd == 1 || fd == 2){
		uint16_t *gs = (void *)0xb8000;
		gs = gs + line * 80 + row;
		union character c;
		int i;
		for(i = 0; i < len; i++){
			c.ch = (buf + 0x200000)[i];
			c.color = 0x0b;
			gs[i] = c.val;
		}
		return len;
	}
	else
		return -1;
}
