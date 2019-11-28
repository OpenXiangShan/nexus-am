/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "misc/config.h"
#include <klib.h>
#include <amdev.h>

/*
todo: rewrite input.  only need a few global input variables:

todo: new input system.  stores input data into the nes struct

required variables:
	mouse x,y
	mouse buttons
	keyboard state
	joystick state (merged into keyboard state, possibly)
*/

//these global variables provide information for the device input code
uint8_t joykeys[370];		//keyboard state
int joyconfig[4][8];	//joypad button configuration

int input_init()
{
	joyconfig[0][0] = config_get_int("input.joypad0.a");
	joyconfig[0][1] = config_get_int("input.joypad0.b");
	joyconfig[0][2] = config_get_int("input.joypad0.select");
	joyconfig[0][3] = config_get_int("input.joypad0.start");
	joyconfig[0][4] = config_get_int("input.joypad0.up");
	joyconfig[0][5] = config_get_int("input.joypad0.down");
	joyconfig[0][6] = config_get_int("input.joypad0.left");
	joyconfig[0][7] = config_get_int("input.joypad0.right");
	joyconfig[1][0] = config_get_int("input.joypad1.a");
	joyconfig[1][1] = config_get_int("input.joypad1.b");
	joyconfig[1][2] = config_get_int("input.joypad1.select");
	joyconfig[1][3] = config_get_int("input.joypad1.start");
	joyconfig[1][4] = config_get_int("input.joypad1.up");
	joyconfig[1][5] = config_get_int("input.joypad1.down");
	joyconfig[1][6] = config_get_int("input.joypad1.left");
	joyconfig[1][7] = config_get_int("input.joypad1.right");

	return(0);
}

void input_kill()
{
}

void input_poll()
{
  int keycode;
  do {
#define KEYDOWN_MASK 0x8000
    int key = read_key();
    keycode = key & ~KEYDOWN_MASK;
    int keydown = (key & KEYDOWN_MASK) != 0;
    switch (keycode) {
      case _KEY_J: joykeys['j'] = keydown; break;
      case _KEY_K: joykeys['k'] = keydown; break;
      case _KEY_U: joykeys['u'] = keydown; break;
      case _KEY_I: joykeys['i'] = keydown; break;
      case _KEY_W: joykeys['w'] = keydown; break;
      case _KEY_S: joykeys['s'] = keydown; break;
      case _KEY_A: joykeys['a'] = keydown; break;
      case _KEY_D: joykeys['d'] = keydown; break;
    }
  } while (keycode != _KEY_NONE);
}

int input_poll_mouse(int *x,int *y)
{
	return 0;
}
