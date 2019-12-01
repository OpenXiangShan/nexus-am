/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "types.h"
#include "file.h"
#include "utils/memory.h"
#include "utils/md5.h"
#include "driver.h"
#include "types.h"
#include "fceu.h"
#include "state.h"
#include "driver.h"

using namespace std;

void FCEU_SplitArchiveFilename(std::string src, std::string& archive, std::string& file, std::string& fileToOpen)
{
	size_t pipe = src.find_first_of('|');
	if(pipe == std::string::npos)
	{
		archive = "";
		file = src;
		fileToOpen = src;
	}
	else
	{
		archive = src.substr(0,pipe);
		file = src.substr(pipe+1);
		fileToOpen = archive;
	}
}

FCEUFILE * FCEU_fopen(const char *path, const char *ipsfn, const char *mode, char *ext, int index, const char** extensions, int* userCancel)
{
	FCEUFILE *fceufp=0;

  assert(ipsfn == NULL);

	bool read = !strcmp(mode, "rb");
	bool write = !strcmp(mode, "wb");
	if((read && write) || (!read && !write))
	{
		FCEU_PrintError("invalid file open mode specified (only wb and rb are supported)");
		return 0;
	}

	std::string archive,fname,fileToOpen;
	FCEU_SplitArchiveFilename(path,archive,fname,fileToOpen);

	if(read)
	{
			//if the archive contained no files, try to open it the old fashioned way
			EMUFILE* fp = FCEUD_UTF8_fstream(fileToOpen.c_str(),mode);
			if(!fp)
				return 0;
			if (!fp->is_open())
			{
				//fp is new'ed so it has to be deleted
				delete fp;
				return 0;
			}

			//open a plain old file
			fceufp = new FCEUFILE();
			fceufp->filename = fileToOpen;
			fceufp->logicalPath = fileToOpen;
			fceufp->fullFilename = fileToOpen;
			fceufp->archiveIndex = -1;
			fceufp->stream = fp;
			FCEU_fseek(fceufp,0,SEEK_END);
			fceufp->size = FCEU_ftell(fceufp);
			FCEU_fseek(fceufp,0,SEEK_SET);

		return fceufp;
	}
	return 0;
}

int FCEU_fclose(FCEUFILE *fp)
{
	delete fp;
	return 1;
}

uint64 FCEU_fread(void *ptr, size_t size, size_t nmemb, FCEUFILE *fp)
{
	return fp->stream->fread((char*)ptr,size*nmemb);
}

int FCEU_fseek(FCEUFILE *fp, long offset, int whence)
{
	fp->stream->fseek(offset,whence);

	return FCEU_ftell(fp);
}

uint64 FCEU_ftell(FCEUFILE *fp)
{
	return fp->stream->ftell();
}
