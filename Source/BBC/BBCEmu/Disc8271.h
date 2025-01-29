/****************************************************************
BeebEm - BBC Micro and Master 128 Emulator
Copyright (C) 1994  David Alan Gilbert
Copyright (C) 1997  Mike Wyatt

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

/* 8271 disc emulation - David Alan Gilbert 4/12/94 */

#ifndef DISC8271_HEADER
#define DISC8271_HEADER

#include "DiscType.h"

extern int Disc8271Trigger; /* Cycle based time Disc8271Trigger */
extern bool Disc8271Enabled;

enum class Disc8271Result {
	Success,
	Failed,
	InvalidFSD,
	InvalidTracks
};

Disc8271Result LoadSimpleDSDiscImage(const char *FileName, int DriveNum, int Tracks);
Disc8271Result LoadSimpleDiscImage(const char *FileName, int DriveNum, int HeadNum, int Tracks);
Disc8271Result LoadFSDDiscImage(const char *FileName, int DriveNum);

bool IsDiscWritable(int Drive);
void DiscWriteEnable(int Drive, bool WriteEnable);
void FreeDiscImage(int Drive);

unsigned char Disc8271Read(int Address);
void Disc8271Write(int Address, unsigned char Value);

void Disc8271_poll_real();

#define Disc8271Poll() if (Disc8271Trigger <= TotalCycles) Disc8271_poll_real();

void Disc8271Reset();

void Save8271UEF(FILE *SUEF);
void Load8271UEF(FILE *SUEF, int Version);

#endif
