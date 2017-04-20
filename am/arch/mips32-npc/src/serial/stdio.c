#include <arch.h>
#include <npc.h>

int curr_line = 0;
int curr_col = 0;

void npc_putc(char ch)
{
	if(ch == '\n'){
		curr_col = 0;
		curr_line += 8;
	}
	else{
//		draw_character(ch,curr_line,curr_col,0x3c);
	}
	if (curr_col + 8 >= SCR_WIDTH) {
			curr_line += 8; curr_col = 0;
	} else {
			curr_col += 8;
	}
	if(curr_line >= SCR_HEIGHT){
//		prepare_buffer();
		curr_line = 0;
	}
}

void npc_puts(const char *s)
{
    while (*s != '\0') {
        npc_putc(*s++);
    }
}

__attribute__((noinline)) void put_hex(unsigned int x, char *addr) {
    int i;
    *addr++ = '0';
    *addr++ = 'x';
    for (i = 0; i < 8; i++) {
        unsigned int val = (x >> ((7 - i) << 2)) & 0xf;
        if (val >= 16) {
            *addr++ = '?';
        }
        else if (val < 10) {
            *addr++ = ((char) val) + '0';
        }
        else {
            *addr++ = ((char) val) - 10 + 'a';
        }
    }
}

static char hex_literals[] = "0123456789abcdef";
#define digit(val, offset) (((val) & (0xfu << ((offset) << 2))) >> ((offset) << 2))

void print_hex(unsigned int x)
{
    npc_putc(hex_literals[ digit(x, 7) ]);
    npc_putc(hex_literals[ digit(x, 6) ]);
    npc_putc(hex_literals[ digit(x, 5) ]);
    npc_putc(hex_literals[ digit(x, 4) ]);
    npc_putc(hex_literals[ digit(x, 3) ]);
    npc_putc(hex_literals[ digit(x, 2) ]);
    npc_putc(hex_literals[ digit(x, 1) ]);
    npc_putc(hex_literals[ digit(x, 0) ]);
}
