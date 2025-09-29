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

#ifndef DISCINFO_HEADER
#define DISCINFO_HEADER

#include "DiscType.h"

struct DiscInfoType
{
    bool Loaded; // Set to true when a disc image has been loaded
    char FileName[256]; // Filename of disc currently in drive 0 and 1
    DiscType Type; // Current disc type
    bool DoubleSidedSSD; // Non-interleaved double sided disk image
};

extern DiscInfoType DiscInfo[2];

bool IsDoubleSidedSSD(const char *FileName, FILE *pFile);

#endif
