/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_FDS.c $
 * $Id: s_FDS.c 376 2008-06-29 20:58:13Z Quietust $
 */

void FDSsound_Load (void) { }
void FDSsound_Reset (void) { }
void FDSsound_Unload (void) { }
int	FDSsound_Read (int Addr) { return -1; }
void	FDSsound_Write (int Addr, int Val) { }
int	FDSsound_Get (int numCycles) { return 0; }
int	FDSsound_SaveLoad (int mode, int x, unsigned char *data) { return x; }
