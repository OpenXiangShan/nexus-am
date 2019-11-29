/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_VRC7.c $
 * $Id: s_VRC7.c 376 2008-06-29 20:58:13Z Quietust $
 */

/***********************************************************************************

	emu2413.c -- YM2413 emulator written by Mitsutaka Okazaki 2001

	2001 01-08 : Version 0.10 -- 1st version.
	2001 01-15 : Version 0.20 -- semi-public version.
	2001 01-16 : Version 0.30 -- 1st public version.
	2001 01-17 : Version 0.31 -- Fixed bassdrum problem.
		   : Version 0.32 -- LPF implemented.
	2001 01-18 : Version 0.33 -- Fixed the drum problem, refine the mix-down method.
				  -- Fixed the LFO bug.
	2001 01-24 : Version 0.35 -- Fixed the drum problem,
				     support undocumented EG behavior.
	2001 02-02 : Version 0.38 -- Improved the performance.
				     Fixed the hi-hat and cymbal model.
				     Fixed the default percussive datas.
				     Noise reduction.
				     Fixed the feedback problem.
	2001 03-03 : Version 0.39 -- Fixed some drum bugs.
				     Improved the performance.
	2001 03-04 : Version 0.40 -- Improved the feedback.
				     Change the default table size.
				     Clock and Rate can be changed during play.
	2001 06-24 : Version 0.50 -- Improved the hi-hat and the cymbal tone.
				     Added VRC7 patch (OPLL_reset_patch is changed).
				     Fixed OPLL_reset() bug.
				     Added OPLL_setMask, OPLL_getMask and OPLL_toggleMask.
				     Added OPLL_writeIO.
	2001 09-28 : Version 0.51 -- Removed the noise table.
	2002 01-28 : Version 0.52 -- Added Stereo mode.
	2002 02-07 : Version 0.53 -- Fixed some drum bugs.
	2002 02-20 : Version 0.54 -- Added the best quality mode.
	2002 03-02 : Version 0.55 -- Removed OPLL_init & OPLL_close.
	2002 05-30 : Version 0.60 -- Fixed HH&CYM generator and all voice datas.

	2003 01-24 : Modified by xodnizel to remove code not needed for the VRC7, among other things.

	References:
		fmopl.c	       -- 1999,2000 written by Tatsuyuki Satoh (MAME development).
		fmopl.c(fixed) -- (C) 2002 Jarek Burczynski.
		s_opl.c	       -- 2001 written by Mamiya (NEZplug development).
		fmgen.cpp      -- 1999,2000 written by cisc.
		fmpac.ill      -- 2000 created by NARUTO.
		MSX-Datapack
		YMU757 data sheet
		YM2143 data sheet

**************************************************************************************/

void	VRC7sound_Load (void) { }
void	VRC7sound_Reset (void) { }
void	VRC7sound_Unload (void) { }
void	VRC7sound_Write (int Addr, int Val) { }
int	VRC7sound_Get (int numCycles) { return 0; }
int	VRC7sound_SaveLoad (int mode, int x, unsigned char *data) { return x; }
