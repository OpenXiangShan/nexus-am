#ifndef common_h
#define common_h

#define HZ 100
#define FPS 30
#define CHARACTER_PER_SECOND 3
#define UPDATE_PER_SECOND 100
#define NULL ((void *)0)

#include <am.h>
#include <klib.h>
#include "linklist.h"

#define SCR_WIDTH (_screen.width)
#define SCR_HEIGHT (_screen.height)

/* 定义fly_t链表 */
LINKLIST_DEF(fly)
	int x;
	int y;
	int text;
	int v;
LINKLIST_DEF_FI(fly)

typedef char bool;

#define true         1
#define false        0

/* 按键相关 */
void press_key(int scan_code);
void release_key(int ch);
bool query_key(int ch);
int last_key_code(void);
bool keyboard_event();

/* 游戏逻辑相关 */
void create_new_letter(void);
void update_letter_pos(void);
bool update_keypress(void);

int get_hit(void);
int get_miss(void);
int get_fps(void);
void set_fps(int fps);
fly_t characters(void);

void redraw_screen();

#endif
