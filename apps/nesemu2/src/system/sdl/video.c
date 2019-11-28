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

#include <klib.h>
#include "misc/log.h"
#include "palette/palette.h"
#include "nes/nes.h"
#include "system/system.h"
#include "system/video.h"
#include "system/sdl/console/console.h"
#include "misc/memutil.h"
#include "misc/config.h"
#include "system/common/filters.h"

//system related variables
static int screenw,screenh,screenbpp;
static int screenscale;

//palette with emphasis applied
static u8 palette[8][64 * 3];

//palette data fed to video system
static u32 palette32[8][256];		//32 bit color

//caches of all available colors
static u16 palettecache16[256];
static u32 palettecache32[256];

//actual values written to nes palette ram
static u8 palettecache[32];

//for frame limiting
static double interval = 0;
static u64 lasttime = 0;

//pointer to scree and copy of the nes screen
static u32 *screen = 0;
static u8 *nesscreen = 0;

//draw function pointer and pointer to current video filter
static void (*drawfunc)(void*,u32,void*,u32,u32,u32);		//dest,destpitch,src,srcpitch,width,height
static filter_t *filter;

//for correct colors
//static int rshift,gshift,bshift;
//static int rloss,gloss,bloss;
#define rshift 16
#define gshift 8
#define bshift 0

static int find_drawfunc(int scale,int bpp)
{
	int i;

	for(i=0;filter->modes[i].scale;i++) {
		if(filter->modes[i].scale == scale) {
			switch(bpp) {
				case 32:
					drawfunc = filter->modes[i].draw32;
					return(0);
				case 16:
				case 15:
				//	drawfunc = filter->modes[i].draw16;
				//	return(0);
				default:
					log_printf("find_drawfunc:  unsupported bit depth (%d)\n",bpp);
					return(2);
			}
		}
	}
	return(1);
}

int video_init()
{
	if(nesscreen == 0)
		nesscreen = (u8*)mem_alloc(256 * (240 + 16));

	//setup timer to limit frames
	interval = (double)system_getfrequency() / 60.0f;
	lasttime = system_gettick();

	//clear palette caches
	memset(palettecache16,0,256*sizeof(u16));
	memset(palettecache32,0,256*sizeof(u32));

	//set screen info
	screenscale = config_get_int("video.scale");

  screenbpp = 32;

	//initialize the video filters
	filter_init();

	//get pointer to video filter
	filter = filter_get((screenscale == 1) ? F_NONE : filter_get_int(config_get_string("video.filter")));

	if(find_drawfunc(screenscale,screenbpp) != 0) {
		log_printf("video_init:  error finding appropriate draw func, using draw1x\n");
		filter = &filter_draw;
		drawfunc = filter->modes[0].draw32;
	}

	//calculate desired screen dimensions
	screenw = filter->minwidth / filter->minscale * screenscale;
	screenh = filter->minheight / filter->minscale * screenscale;

  screenw = screen_width();
  screenh = screen_height();

	//allocate memory for temp screen buffer
	screen = (u32*)mem_realloc(screen,256 * (240 + 16) * (screenbpp / 8) * 4);

	return(0);
}

void video_kill()
{
	filter_kill();
	if(screen)
		mem_free(screen);
	if(nesscreen)
		mem_free(nesscreen);
	screen = 0;
	nesscreen = 0;
}

int video_reinit()
{
	video_kill();
	return(video_init());
}

void video_startframe()
{
}

void video_endframe()
{
	u64 t;

	//draw everything
  draw_rect(screen, 0, 0, 256, 240);
	console_draw(screen, 256*4,screenh);

	//simple frame limiter
	if(config_get_bool("video.framelimit")) {
		do {
			t = system_gettick();
		} while((double)(t - lasttime) < interval);
		lasttime = t;
	}
}

//this handles lines for gui/status messages
void video_updateline(int line,u8 *s)
{
	u32 *dest = screen + (line * 256);
	int i;

	memcpy(nesscreen + (line * 256),s,256);
	if(line >= 8 && line < 232) {
		for(i=0;i<256;i++) {
			*dest++ = palettecache32[*s++];
		}
	}
	else {
		for(i=0;i<256;i++) {
			*dest++ = 0;
		}
	}
}

//this handles pixels coming directly from the nes engine
void video_updatepixel(int line,int pixel,u8 s)
{
	int offset = (line * 256) + pixel;

	nesscreen[offset] = s;
	if(line >= 8 && line < 232) {
		screen[offset] = palettecache32[s];
	}
	else {
		screen[offset] = 0;
	}
}

//this handles palette changes from the nes engine
void video_updatepalette(u8 addr,u8 data)
{
	palettecache32[addr+0x00] = palette32[0][data];
	palettecache32[addr+0x20] = palette32[1][data];
	palettecache32[addr+0x40] = palette32[2][data];
	palettecache32[addr+0x60] = palette32[3][data];
	palettecache32[addr+0x80] = palette32[4][data];
	palettecache32[addr+0xA0] = palette32[5][data];
	palettecache32[addr+0xC0] = palette32[6][data];
	palettecache32[addr+0xE0] = palette32[7][data];
	palettecache[addr] = data;
}

//must be called AFTER video_init
void video_setpalette(palette_t *p)
{
	int i,j;
	palentry_t *e;

	for(j=0;j<8;j++) {
		for(i=0;i<64;i++) {
			palette[j][(i * 3) + 0] = p->pal[j][i].r;
			palette[j][(i * 3) + 1] = p->pal[j][i].g;
			palette[j][(i * 3) + 2] = p->pal[j][i].b;
		}
	}

	for(j=0;j<8;j++) {
		for(i=0;i<256;i++) {
			e = &p->pal[j][i & 0x3F];
			palette32[j][i] = (e->r << rshift) | (e->g << gshift) | (e->b << bshift);
		}
	}

	filter_palette_changed();
}

int video_getwidth()				{	return(screenw);			}
int video_getheight()			{	return(screenh);			}
int video_getbpp()				{	return(screenbpp);		}
u8 *video_getscreen()			{	return(nesscreen);		}
u8 *video_getpalette()			{	return((u8*)palette);	}

int video_zapperhit(int x,int y)
{
	int ret = 0;
	u8 *e;
	u8 color;

	color = palettecache[nesscreen[x + y * 256]];
	e = &palette[(color >> 5) & 7][(color & 0x3F) * 3];
	ret += (int)(e[0] * 0.299f);
	ret += (int)(e[1] * 0.587f);
	ret += (int)(e[2] * 0.114f);
	return((ret >= 0x40) ? 1 : 0);
}

//kludge-city!
int video_getxoffset()	{	return(0);	}
int video_getyoffset()	{	return(0);	}
int video_getscale()		{	return(screenscale);	}
