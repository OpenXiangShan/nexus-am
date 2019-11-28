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

#ifndef __fds__hle_h__
#define __fds__hle_h__

#define log_hle if(hlelog) log_printf

extern u8 hleregs[];
extern u8 hlemem[];
extern int hlelog;

//helper functions
u32 hle_getparam(int n);
void hle_fixretaddr(int n);

//nes mapper functions
u8 hlefds_read(u32 addr);
void hlefds_write(u32 addr,u8 data);
void hlefds_cpucycle();

//for replacing real bios calls with the hle calls
void hlefds_intercept();

#endif
