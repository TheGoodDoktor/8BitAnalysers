/****************************************************************
BeebEm - BBC Micro and Master 128 Emulator
Copyright (C) 2009  Mike Wyatt

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public 
License along with this program; if not, write to the Free 
Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA  02110-1301, USA.
****************************************************************/

#include <stdio.h>

#include "DiscInfo.h"
#include "DiscEdit.h"
//#include "FileUtils.h"

DiscInfoType DiscInfo[2];

bool IsDoubleSidedSSD(const char *FileName, FILE *pFile)
{
#ifdef BBCA_TODO
	if (!HasFileExt(FileName, ".ssd"))
	{
		return false;
	}
#endif

	fseek(pFile, 0, SEEK_END);

	long Size = ftell(pFile);

	fseek(pFile, 0, SEEK_SET);

	return Size > 80 * DFS_SECTORS_PER_TRACK * DFS_SECTOR_SIZE;
}
