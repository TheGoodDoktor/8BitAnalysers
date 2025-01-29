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

//
// BeebEm Disc Edit - file import/export
//
// Mike Wyatt - Mar 2009
//

#ifndef DISCEDIT_HEADER
#define DISCEDIT_HEADER

#include <string>

constexpr int DFS_MAX_CAT_SIZE       = 62; // Allow for Watford DFS
constexpr int DFS_MAX_TITLE_LEN      = 12;
constexpr int DFS_MAX_NAME_LEN       = 7;
constexpr int DFS_SECTORS_PER_TRACK  = 10;
constexpr int DFS_SECTOR_SIZE        = 256;

struct DFS_FILE_ATTR
{
	char filename[DFS_MAX_NAME_LEN + 1];
	char directory;
	bool locked;
	int loadAddr;
	int execAddr;
	int length;
	int startSector;
};

struct DFS_DISC_CATALOGUE
{
	char title[DFS_MAX_TITLE_LEN + 1];
	int numWrites;
	int numFiles;
	int numSectors;
	int bootOpts;
	bool watford62;
	DFS_FILE_ATTR fileAttrs[DFS_MAX_CAT_SIZE];
};

std::string BeebToLocalFileName(const std::string& BeebFileName);

bool dfs_get_catalogue(const char *szDiscFile,
                       int numSides,
                       int side,
                       DFS_DISC_CATALOGUE *dfsCat);

bool dfs_export_file(const char *szDiscFile,
                     int numSides,
                     int side,
                     const DFS_FILE_ATTR* attr,
                     const char *szExportFolder,
                     char *szErrStr);

bool dfs_import_file(const char *szDiscFile,
                     int numSides,
                     int side,
                     DFS_DISC_CATALOGUE *dfsCat,
                     const char *szFile,
                     const char *szImportFolder,
                     char *szErrStr);

#endif
