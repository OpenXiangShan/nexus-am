#include "common.h"

/* a-z对应的键盘扫描码 */
static int letter_code[] = {
	0x1c,0x32,0x21,0x23,0x24,
	0x2b,0x34,0x33,0x43,0x3b,
	0x42,0x4b,0x3a,0x31,0x44,
	0x4d,0x15,0x2d,0x1b,0x2c,
	0x3c,0x2a,0x1d,0x22,0x35,0x1a
};
/* 对应键按下的标志位 */
static bool letter_pressed[26];

void
press_key(int scan_code) {
	int i;
	for (i = 0; i < 26; i ++) {
		if (letter_code[i] == scan_code) {
			letter_pressed[i] = true;
		}
	}
}

void
release_key(int index) {
	letter_pressed[index] = false;
}

bool
query_key(int index) {
	return letter_pressed[index];
}

/* key_code保存了上一次键盘事件中的扫描码 */
static volatile int key_code = 0;

int last_key_code(void) {
	return key_code;
}

bool
keyboard_event() {
	//TODO:listen keyboard,catch scan_code assigning to key_code
	key_code = _peek_key();
	if(key_code == _KEY_DOWN){
		press_key(key_code);
		return true;
	}
	return false;
}

