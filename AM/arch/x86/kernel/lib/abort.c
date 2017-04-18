#include "common.h"
#include "x86.h"
#include "device.h"

static char *
itoa(int a) {
	static char buf[30];
	char *p = buf + sizeof(buf) - 1;
	do {
		*--p = '0' + a % 10;
	} while (a /= 10);
	return p;
}

static void
append(char **p, const char *str) {
	while (*str) {
		*((*p) ++) = *str ++;
	}
}

/* 将文件名和assert fail的行号显示在屏幕上 */
#define BLUE_SCREEN_TEXT "Assertion failed: "
static void
display_message(const char *file, int line) {
	static char buf[256] = BLUE_SCREEN_TEXT;
	char *p = buf + sizeof(BLUE_SCREEN_TEXT) - 1;

	append(&p, file);
	append(&p, ":");
	append(&p, itoa(line));
	append(&p, "\n");

	for (p = buf; *p; p ++) {
		putchar(*p);
	}
}

int
abort(const char *fname, int line) {
	disable_interrupt();
	display_message(fname, line);
	while (TRUE) {
		wait_for_interrupt();
	}
}
