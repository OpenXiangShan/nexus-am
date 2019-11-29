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

#include "emu/emu.h"
#include "nes/nes.h"
#include "system/main.h"

int main(const char *romfilename) {
  _ioe_init();

	//initialize the emulator
	if (emu_init() != 0) {
    log_printf("main:  emu_init() failed\n");
    return(2);
	}

  int ret = nes_load((char*)romfilename);
  assert(ret == 0);
  nes_reset(1);
  running = 1;

	//begin the main loop
	ret = emu_mainloop();

	//destroy emulator
	emu_kill();

	//return to os
	return (emu_exit(ret));
}
