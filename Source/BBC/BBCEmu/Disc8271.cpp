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

// 04/12/1994 David Alan Gilbert: 8271 disc emulation
// 30/08/1997 Mike Wyatt: Added disc write and format support
// 27/12/2011 J.G.Harston: Double-sided SSD supported

//#include <windows.h>

#include <fstream>

#include "Disc8271.h"
//#include "6502core.h"
//#include "DebugTrace.h"
#include "DiscEdit.h"
#include "DiscInfo.h"
#include "DiscType.h"
//#include "Log.h"
//#include "Main.h"
//#include "Sound.h"
//#include "StringUtils.h"
//#include "SysVia.h"
//#include "Tube.h"
//#include "UefState.h"

#include "BBCEmu.h"

// #define DEBUG_8271

#define ENABLE_LOG 0

// 8271 Status register
const unsigned char STATUS_REG_COMMAND_BUSY       = 0x80;
const unsigned char STATUS_REG_COMMAND_FULL       = 0x40;
const unsigned char STATUS_REG_PARAMETER_FULL     = 0x20;
const unsigned char STATUS_REG_RESULT_FULL        = 0x10;
const unsigned char STATUS_REG_INTERRUPT_REQUEST  = 0x08;
const unsigned char STATUS_REG_NON_DMA_MODE       = 0x04;

// 8271 Result register
const unsigned char RESULT_REG_SUCCESS                = 0x00;
const unsigned char RESULT_REG_SCAN_NOT_MET           = 0x00;
const unsigned char RESULT_REG_SCAN_MET_EQUAL         = 0x02;
const unsigned char RESULT_REG_SCAN_MET_NOT_EQUAL     = 0x04;
const unsigned char RESULT_REG_CLOCK_ERROR            = 0x08;
const unsigned char RESULT_REG_LATE_DMA               = 0x0A;
const unsigned char RESULT_REG_ID_CRC_ERROR           = 0x0C;
const unsigned char RESULT_REG_DATA_CRC_ERROR         = 0x0E;
const unsigned char RESULT_REG_DRIVE_NOT_READY        = 0x10;
const unsigned char RESULT_REG_WRITE_PROTECT          = 0x12;
const unsigned char RESULT_REG_TRACK_0_NOT_FOUND      = 0x14;
const unsigned char RESULT_REG_WRITE_FAULT            = 0x16;
const unsigned char RESULT_REG_SECTOR_NOT_FOUND       = 0x18;
const unsigned char RESULT_REG_DRIVE_NOT_PRESENT      = 0x1E; // Undocumented, see http://beebwiki.mdfs.net/OSWORD_%267F
const unsigned char RESULT_REG_DELETED_DATA_FOUND     = 0x20;
const unsigned char RESULT_REG_DELETED_DATA_CRC_ERROR = 0x2E;

// 8271 special registers
const unsigned char SPECIAL_REG_SCAN_SECTOR_NUMBER        = 0x06;
const unsigned char SPECIAL_REG_SCAN_COUNT_MSB            = 0x14;
const unsigned char SPECIAL_REG_SCAN_COUNT_LSB            = 0x13;
const unsigned char SPECIAL_REG_SURFACE_0_CURRENT_TRACK   = 0x12;
const unsigned char SPECIAL_REG_SURFACE_1_CURRENT_TRACK   = 0x1A;
const unsigned char SPECIAL_REG_MODE_REGISTER             = 0x17;
const unsigned char SPECIAL_REG_DRIVE_CONTROL_OUTPUT_PORT = 0x23;
const unsigned char SPECIAL_REG_DRIVE_CONTROL_INPUT_PORT  = 0x22;
const unsigned char SPECIAL_REG_SURFACE_0_BAD_TRACK_1     = 0x10;
const unsigned char SPECIAL_REG_SURFACE_0_BAD_TRACK_2     = 0x11;
const unsigned char SPECIAL_REG_SURFACE_1_BAD_TRACK_1     = 0x18;
const unsigned char SPECIAL_REG_SURFACE_1_BAD_TRACK_2     = 0x19;

bool Disc8271Enabled = true;
int Disc8271Trigger; // Cycle based time Disc8271Trigger

constexpr int TRACKS_PER_DRIVE = 80;

// Note: reads/writes one byte every 80us
constexpr int TIME_BETWEEN_BYTES = 160;

struct IDFieldType {
	// Cylinder Number byte which identifies the track number
	unsigned char LogicalTrack;
	// Head Number byte which specifies the head used (top or bottom)
	// to access the sector: 0 for single sided discs and side 0 of
	// two sided discs, and 1 for side 1 of two sided discs
	unsigned char HeadNum;
	// Record Number byte identifying the sector number
	unsigned char LogicalSector;
	// The byte length of the sector (Physical Record Length in the 8271 datasheet)
	// 0: 128 bytes, 1: 256 bytes, 2: 512 bytes
	unsigned char SectorLength;
};

struct SectorType {
	IDFieldType IDField;
	unsigned char CylinderNum; // Physical track number
	unsigned char RecordNum; // Physical sector number
	unsigned char Error; // FSD - error code when sector was read, 20 for deleted data
	int RealSectorSize; // The actual size of the sector, in bytes
	unsigned char *Data;
};

struct TrackType {
	int LogicalSectors; // Number of sectors in the current track (read from FSD file)
	int NSectors; // Number of sectors in the Sectors array
	SectorType *Sectors;
	int Gap1Size; // From Format command
	int Gap3Size;
	int Gap5Size;
	bool TrackIsReadable; // FSD - is the track readable, or just contains track ID?
};

struct DiscStatusType {
	bool Writeable; // True if the disc is writeable
	int NumHeads; // Number of sides of loaded disc images
	int TotalTracks; // Total number of tracks in FSD disk image
	TrackType Tracks[2][TRACKS_PER_DRIVE]; // All data on the disc - first param head, then physical track ID
};

static DiscStatusType DiscStatus[2];

struct FDCStateType {
	unsigned char ResultReg;
	unsigned char StatusReg;
	unsigned char DataReg;

	unsigned char Command;
	int CommandParamCount;
	int CurrentParam; // From 0
	unsigned char Params[16]; // Wildly more than we need

	// These bools indicate which drives the last command selected.
	// They also act as "drive ready" bits which are reset when the motor stops.
	bool Select[2]; // Drive selects

	unsigned char DriveHeadPosition[2];
	bool DriveHeadLoaded;
	bool DriveHeadUnloadPending;

	unsigned char PositionInTrack[2]; // FSD
	bool SectorOverRead; // FSD - Was read size bigger than data stored?
	bool UsingSpecial; // FSD - Using Special Register
	unsigned char DRDSC; // FSD
	unsigned char FSDLogicalTrack[2];
	unsigned char FSDPhysicalTrack[2];

	unsigned char ScanSectorNum;
	unsigned int ScanCount; // Read as two bytes
	unsigned char ModeReg;
	unsigned char CurrentTrack[2]; // 0/1 for surface number
	unsigned char DriveControlOutputPort;
	unsigned char DriveControlInputPort;
	unsigned char BadTracks[2][2]; // 1st subscript is surface 0/1 and second subscript is badtrack 0/1

	// State set by the Specify (initialisation) command
	// See Intel 8271 data sheet, page 15, ADUG page 39-40
	int StepRate; // In 2ms steps
	int HeadSettlingTime; // In 2ms steps
	int IndexCountBeforeHeadUnload; // Number of revolutions (0 to 14), or 15 to keep loaded
	int HeadLoadTime; // In 8ms steps
};

static FDCStateType FDCState;

/*--------------------------------------------------------------------------*/

struct CommandStatusType {
	unsigned char TrackAddr;
	unsigned char CurrentSector;
	int SectorLength; // In bytes
	int SectorsToGo;

	SectorType *CurrentSectorPtr;
	TrackType *CurrentTrackPtr;

	int ByteWithinSector; // Next byte in sector or ID field
	bool FirstWriteInt; // Indicates the start of a write operation
	unsigned char NextInterruptIsErr; // non-zero causes error and drops this value into result reg
};

static CommandStatusType CommandStatus;

/*--------------------------------------------------------------------------*/

// Note Head select is done from bit 5 of the drive output register
#define CURRENT_HEAD ((FDCState.DriveControlOutputPort >> 5) & 1)

static bool SaveTrackImage(int DriveNum, int HeadNum, int TrackNum);
static void DriveHeadScheduleUnload();
static unsigned short GetFSDSectorSize(unsigned char Index);

/*--------------------------------------------------------------------------*/

static void UpdateNMIStatus()
{
	if (FDCState.StatusReg & STATUS_REG_INTERRUPT_REQUEST)
	{
		NMIStatus |= 1 << nmi_floppy;
	}
	else
	{
		NMIStatus &= ~(1 << nmi_floppy);
	}
}

/*--------------------------------------------------------------------------*/

// For appropriate commands checks the select bits in the command code and
// selects the appropriate drive.

static void DoSelects()
{
	FDCState.Select[0] = (FDCState.Command & 0x40) != 0;
	FDCState.Select[1] = (FDCState.Command & 0x80) != 0;

	FDCState.DriveControlOutputPort &= 0x3f;

	if (FDCState.Select[0]) FDCState.DriveControlOutputPort |= 0x40;
	if (FDCState.Select[1]) FDCState.DriveControlOutputPort |= 0x80;
}

/*--------------------------------------------------------------------------*/

static void NotImp(const char *Command)
{
	//TODO: mainWin->Report(MessageType::Error,
	//                "Disc operation '%s' not supported", Command);
}

/*--------------------------------------------------------------------------*/

// Load the head - ignore for the moment

static void DoLoadHead()
{
}

/*--------------------------------------------------------------------------*/

// Initialise our disc structures

static void InitDiscStore()
{
	const TrackType Blank = { 0, 0, nullptr, 0, 0, 0, false };

	for (int Drive = 0; Drive < 2; Drive++)
	{
		for (int Head = 0; Head < 2; Head++)
		{
			for (int Track = 0; Track < TRACKS_PER_DRIVE; Track++)
			{
				DiscStatus[Drive].Tracks[Head][Track] = Blank;
			}
		}
	}
}

/*--------------------------------------------------------------------------*/

// Given a logical track number accounts for bad tracks

static unsigned char SkipBadTracks(int Drive, unsigned char Track)
{
	if (DiscInfo[Drive].Type == DiscType::FSD)
	{
		return Track; // FSD - no bad tracks, but possible to have unformatted
	}
	else
	{
		int Offset = 0;

#ifdef BBCA_TODO
		if (TubeType != TubeDevice::TorchZ80) // If running under Torch Z80, ignore bad tracks
		{
			if (FDCState.BadTracks[Drive][0] <= Track) Offset++;
			if (FDCState.BadTracks[Drive][1] <= Track) Offset++;
		}
#endif
		return (unsigned char)(Track + Offset);
	}
}

/*--------------------------------------------------------------------------*/

static int GetSelectedDrive()
{
	if (FDCState.Select[0])
	{
		return 0;
	}

	if (FDCState.Select[1])
	{
		return 1;
	}

	return -1;
}

/*--------------------------------------------------------------------------*/

// Returns a pointer to the data structure for a particular track. You pass
// the logical track number, it takes into account bad tracks and the drive
// select and head select etc.  It always returns a valid ptr - if there
// aren't that many tracks then it uses the last one.
// The one exception!!!! is that if no drives are selected it returns nullptr
// FSD - returns the physical track pointer for track ID

static TrackType *GetTrackPtrPhysical(unsigned char PhysicalTrackID)
{
	int Drive = GetSelectedDrive();

	if (Drive < 0) return nullptr;

	FDCState.PositionInTrack[Drive] = 0;
	FDCState.FSDPhysicalTrack[Drive] = PhysicalTrackID;

	return &DiscStatus[Drive].Tracks[CURRENT_HEAD][PhysicalTrackID];
}

/*--------------------------------------------------------------------------*/

// Returns a pointer to the data structure for a particular track.  You pass
// the logical track number, it takes into account bad tracks and the drive
// select and head select etc.  It always returns a valid ptr - if there
// there aren't that many tracks then it uses the last one.
// The one exception!!!! is that if no drives are selected it returns nullptr

// FSD - unsigned char because maximum &FF

static TrackType *GetTrackPtr(unsigned char LogicalTrackID)
{
	int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		return nullptr;
	}

	if (DiscInfo[Drive].Type == DiscType::FSD)
	{
		// Read two tracks extra
		for (unsigned char Track = FDCState.FSDPhysicalTrack[Drive]; Track < FDCState.FSDPhysicalTrack[Drive] +  2; Track++)
		{
			SectorType *SecPtr = DiscStatus[Drive].Tracks[CURRENT_HEAD][Track].Sectors;

			// Fixes Krakout!
			if (SecPtr == nullptr)
			{
				return nullptr;
			}

			if (LogicalTrackID == SecPtr[0].IDField.LogicalTrack)
			{
				FDCState.FSDPhysicalTrack[Drive] = Track;
				return &DiscStatus[Drive].Tracks[CURRENT_HEAD][Track];
			}
		}

		return nullptr; // if it's not found from the above, then it doesn't exist!
	}
	else
	{
		LogicalTrackID = SkipBadTracks(Drive, LogicalTrackID);

		if (LogicalTrackID >= TRACKS_PER_DRIVE)
		{
			LogicalTrackID = TRACKS_PER_DRIVE - 1;
		}

		return &DiscStatus[Drive].Tracks[CURRENT_HEAD][LogicalTrackID];
	}
}

/*--------------------------------------------------------------------------*/

// Returns a pointer to the data structure for a particular sector.
// Returns nullptr for Sector not found. Doesn't check cylinder/head ID.

static SectorType *GetSectorPtr(TrackType *Track, unsigned char LogicalSectorID, bool /* FindDeleted */)
{
	int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		return nullptr;
	}

	if (Track->Sectors == nullptr)
	{
		return nullptr;
	}

	// FSD - from PositionInTrack, instead of 0 to allow Mini Office II to have repeated sector ID
	// if logical sector from track ID is logicalsectorid passed here then return the record number
	// and move the positionintrack to here too

	for (int CurrentSector = FDCState.PositionInTrack[Drive]; CurrentSector < Track->NSectors; CurrentSector++)
	{
		if (Track->Sectors[CurrentSector].IDField.LogicalSector == LogicalSectorID)
		{
			LogicalSectorID = Track->Sectors[CurrentSector].RecordNum;
			FDCState.PositionInTrack[Drive] = Track->Sectors[CurrentSector].RecordNum;
			return &Track->Sectors[LogicalSectorID];
		}
	}

	// As above, but from sector 0 to the current position
	if (FDCState.PositionInTrack[Drive] > 0)
	{
		for (unsigned char CurrentSector = 0; CurrentSector < FDCState.PositionInTrack[Drive]; CurrentSector++)
		{
			if (Track->Sectors[CurrentSector].IDField.LogicalSector == LogicalSectorID)
			{
				LogicalSectorID = Track->Sectors[CurrentSector].RecordNum;
				FDCState.PositionInTrack[Drive] = CurrentSector;
				return &Track->Sectors[LogicalSectorID];
			}
		}
	}

	return nullptr;
}

/*--------------------------------------------------------------------------*/

// Returns a pointer to the data structure for a particular sector.
// Returns nullptr for Sector not found. Doesn't check cylinder/head ID
// FSD - returns the sector IDs

static SectorType *GetSectorPtrForTrackID(TrackType *Track, unsigned char LogicalSectorID, bool /* FindDeleted */)
{
	int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		return nullptr;
	}

	if (Track->Sectors == nullptr)
	{
		return nullptr;
	}

	LogicalSectorID = Track->Sectors[FDCState.PositionInTrack[Drive]].RecordNum;

	return &Track->Sectors[LogicalSectorID];
}

/*--------------------------------------------------------------------------*/

// Cause an error - pass err num

static void DoErr(unsigned char ErrNum)
{
	SetTrigger(50, Disc8271Trigger); // Give it a bit of time
	CommandStatus.NextInterruptIsErr = ErrNum;
	FDCState.StatusReg = STATUS_REG_COMMAND_BUSY; // Command is busy - come back when I have an interrupt
	UpdateNMIStatus();
}

/*--------------------------------------------------------------------------*/

// Checks a few things in the sector - returns true if OK
// FSD - Sectors are always OK

static bool ValidateSector(const SectorType* /*Sector */, int /* Track */, int /* SecLength */)
{
	return true;
}

/*--------------------------------------------------------------------------*/

// The Scan Data Command is used to search for a specific pattern or "key"
// from memory. The 8271 FDC operation is unique in that data is read from
// memory and from the diskette simultaneously.
//
// Parameters:
// 0: Logical Track Address (0-255)
// 1: Logical Sector Address (0-255)
// 2: Sector Size / Number of Sectors
//    (bit 7-bit 5) determine the length of the disk record
//    0 0 0  128 bytes
//    0 0 1  256 bytes
//    0 1 0  512 bytes
//    0 1 1  1024 bytes
//    1 0 0  2048 bytes
//    1 0 1  4096 bytes
//    1 1 0  8192 bytes
//    1 1 1  16384 bytes

static void DoVarLength_ScanDataCommand()
{
	DoSelects();
	NotImp("DoVarLength_ScanDataCommand");
}

/*--------------------------------------------------------------------------*/

// The Scan Data and Deleted Data Command is used to search for a specific
// pattern or "key" from memory. The 8271 FDC operation is unique in that
// data is read from memory and from the diskette simultaneously.
//
// Parameters:
// 0: Logical Track Address (0-255)
// 1: Logical Sector Address (0-255)
// 2: Sector Size / Number of Sectors
//    (bit 7-bit 5) determine the length of the disk record
//    0 0 0  128 bytes
//    0 0 1  256 bytes
//    0 1 0  512 bytes
//    0 1 1  1024 bytes
//    1 0 0  2048 bytes
//    1 0 1  4096 bytes
//    1 1 0  8192 bytes
//    1 1 1  16384 bytes

static void DoVarLength_ScanDataAndDeldCommand()
{
	DoSelects();
	NotImp("DoVarLength_ScanDataAndDeldCommand");
}

/*--------------------------------------------------------------------------*/

// 128 Byte Single Record Write Data Command
//
// Parameters:
// 0: Track Address (0-255)
// 1: Sector (0-255)

static void Do128ByteSR_WriteDataCommand()
{
	DoSelects();
	NotImp("Do128ByteSR_WriteDataCommand");
}

/*--------------------------------------------------------------------------*/

// Write Data Command
//
// Parameters:
// 0: Logical Track Address (0-255)
// 1: Logical Sector Address (0-255)
// 2: Sector Size / Number of Sectors
//    (bit 7-bit 5) determine the length of the disk record
//    0 0 0  128 bytes
//    0 0 1  256 bytes
//    0 1 0  512 bytes
//    0 1 1  1024 bytes
//    1 0 0  2048 bytes
//    1 0 1  4096 bytes
//    1 1 0  8192 bytes
//    1 1 1  16384 bytes

static void DoVarLength_WriteDataCommand()
{
	DoSelects();
	DoLoadHead();

	const int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	if (!DiscStatus[Drive].Writeable)
	{
		DoErr(RESULT_REG_WRITE_PROTECT);
		return;
	}

	FDCState.CurrentTrack[Drive] = FDCState.Params[0];
	CommandStatus.CurrentTrackPtr = GetTrackPtr(FDCState.Params[0]);

	if (CommandStatus.CurrentTrackPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr, FDCState.Params[1], false);

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
		return;
	}

	CommandStatus.TrackAddr     = FDCState.Params[0];
	CommandStatus.CurrentSector = FDCState.Params[1];
	CommandStatus.SectorsToGo   = FDCState.Params[2] & 0x1F;
	CommandStatus.SectorLength  = 1 << (7 + ((FDCState.Params[2] >> 5) & 7));

	if (ValidateSector(CommandStatus.CurrentSectorPtr, CommandStatus.TrackAddr, CommandStatus.SectorLength))
	{
		CommandStatus.ByteWithinSector = 0;
		CommandStatus.FirstWriteInt = true;

		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY;
		UpdateNMIStatus();

		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
	else
	{
		DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
	}
}

/*--------------------------------------------------------------------------*/

static void WriteInterrupt()
{
	bool LastByte = false;

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	if (CommandStatus.SectorsToGo < 0)
	{
		FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
		UpdateNMIStatus();
		return;
	}

	if (!CommandStatus.FirstWriteInt)
	{
		CommandStatus.CurrentSectorPtr->Data[CommandStatus.ByteWithinSector++] = FDCState.DataReg;
	}
	else
	{
		CommandStatus.FirstWriteInt = false;
	}

	FDCState.ResultReg = RESULT_REG_SUCCESS;

	if (CommandStatus.ByteWithinSector >= CommandStatus.SectorLength)
	{
		CommandStatus.ByteWithinSector = 0;

		if (--CommandStatus.SectorsToGo > 0)
		{
			CommandStatus.CurrentSector++;
			CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr,
			                                              CommandStatus.CurrentSector,
			                                              false);

			if (CommandStatus.CurrentSectorPtr == nullptr)
			{
				DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
				return;
			}
		}
		else
		{
			// Last sector done, write the track back to disc
			if (SaveTrackImage(FDCState.Select[0] ? 0 : 1, CURRENT_HEAD, CommandStatus.TrackAddr))
			{
				LastByte = true;
				CommandStatus.SectorsToGo = -1; // To let us bail out

				FDCState.StatusReg = STATUS_REG_RESULT_FULL;
				UpdateNMIStatus();

				SetTrigger(0, Disc8271Trigger); // To pick up result
			}
			else
			{
				DoErr(RESULT_REG_WRITE_PROTECT);
			}
		}
	}

	if (!LastByte)
	{
		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
		                     STATUS_REG_INTERRUPT_REQUEST |
		                     STATUS_REG_NON_DMA_MODE;
		UpdateNMIStatus();
		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
}

/*--------------------------------------------------------------------------*/

// 128 Byte Single Record Write Deleted Data Command
//
// Parameters:
// 0: Track Address (0-255)
// 1: Sector (0-255)

static void Do128ByteSR_WriteDeletedDataCommand()
{
	DoSelects();
	NotImp("Do128ByteSR_WriteDeletedDataCommand");
}

/*--------------------------------------------------------------------------*/

// Write Deleted Data Command
//
// Parameters:
// 0: Logical Track Address (0-255)
// 1: Logical Sector Address (0-255)
// 2: Sector Size / Number of Sectors
//    (bit 7-bit 5) determine the length of the disk record
//    0 0 0  128 bytes
//    0 0 1  256 bytes
//    0 1 0  512 bytes
//    0 1 1  1024 bytes
//    1 0 0  2048 bytes
//    1 0 1  4096 bytes
//    1 1 0  8192 bytes
//    1 1 1  16384 bytes

static void DoVarLength_WriteDeletedDataCommand()
{
	DoSelects();
	NotImp("DoVarLength_WriteDeletedDataCommand");
}

/*--------------------------------------------------------------------------*/

// 128 Byte Single Record Read Data Command
//
// Parameters:
// 0: Track Address (0-255)
// 1: Sector (0-255)

static void Do128ByteSR_ReadDataCommand()
{
	DoSelects();
	NotImp("Do128ByteSR_ReadDataCommand");
}

/*--------------------------------------------------------------------------*/

// The Read Data command transfers data from a specified disk record or
// group of records to memory.
//
// Parameters:
// 0: Logical Track Address (0-255)
// 1: Logical Sector Address (0-255)
// 2: Sector Size / Number of Sectors
//    (bit 7-bit 5) determine the length of the disk record
//    0 0 0  128 bytes
//    0 0 1  256 bytes
//    0 1 0  512 bytes
//    0 1 1  1024 bytes
//    1 0 0  2048 bytes
//    1 0 1  4096 bytes
//    1 1 0  8192 bytes
//    1 1 1  16384 bytes

static void DoVarLength_ReadDataCommand()
{
	DoSelects();
	DoLoadHead();

	#ifdef DEBUG_8271
	DebugTrace("8271: Read Data (Variable Length), Track %d Sector %d, Sectors %d, SectorLength %d\n",
	           FDCState.Params[0],
	           FDCState.Params[1],
	           FDCState.Params[2] & 0x1F,
	           1 << (7 + ((FDCState.Params[2] >> 5) & 7)));
	#endif

	FDCState.SectorOverRead = false; // FSD - if read size was larger than data stored

	const int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	// Reset shift state if it was set by Run Disc
#ifdef BBCA_TODO
	if (mainWin->m_ShiftBooted)
	{
		mainWin->m_ShiftBooted = false;
		BeebKeyUp(0, 0);
	}
#endif

	// FSD - if special register is NOT being used to point to track
	if (!FDCState.UsingSpecial)
	{
		FDCState.FSDPhysicalTrack[Drive] = FDCState.Params[0];
	}

	// if reading a new track, then reset position
	if (FDCState.FSDLogicalTrack[Drive] != FDCState.Params[0])
	{
		FDCState.PositionInTrack[Drive] = 0;
	}

	FDCState.FSDLogicalTrack[Drive] = FDCState.Params[0];

	if (FDCState.DRDSC > 1)
	{
		FDCState.FSDPhysicalTrack[Drive] = 0; // FSDLogicalTrack
	}

	FDCState.DRDSC = 0;

	/* if (FSDLogicalTrack == 0)
	{
		FSDPhysicalTrack = 0;
	} */

	if (FDCState.FSDPhysicalTrack[Drive] == 0)
	{
		FDCState.FSDPhysicalTrack[Drive] = FDCState.FSDLogicalTrack[Drive];
	}

	// fixes The Music System
	if (FDCState.FSDLogicalTrack[Drive] == FDCState.FSDPhysicalTrack[Drive])
	{
		FDCState.UsingSpecial = false;
	}

	CommandStatus.CurrentTrackPtr = GetTrackPtr(FDCState.FSDLogicalTrack[Drive]);

	// Internal_CurrentTrack[Drive] = Params[0];
	// CommandStatus.CurrentTrackPtr = GetTrackPtr(Params[0]);

	if (CommandStatus.CurrentTrackPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	// FSD - if track contains no data
	if (!CommandStatus.CurrentTrackPtr->TrackIsReadable)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr, FDCState.Params[1], false);

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	// (Over)Reading Track 2, Sector 9 on 3D Pool should result in Sector Not Found
	if ((CommandStatus.CurrentSectorPtr->Error == 0xE0) &&
	    (CommandStatus.CurrentSectorPtr->IDField.LogicalSector == 0x09) &&
	    (CommandStatus.SectorLength > CommandStatus.CurrentSectorPtr->RealSectorSize))
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	CommandStatus.TrackAddr     = FDCState.Params[0];
	CommandStatus.CurrentSector = FDCState.Params[1];
	CommandStatus.SectorsToGo   = FDCState.Params[2] & 0x1F;
	CommandStatus.SectorLength  = 1 << (7 + ((FDCState.Params[2] >> 5) & 7));

	// FSD - if trying to read more data than is stored, Disc Duplicator 3
	if (CommandStatus.SectorLength > CommandStatus.CurrentSectorPtr->RealSectorSize)
	{
		CommandStatus.SectorLength = CommandStatus.CurrentSectorPtr->RealSectorSize;
		FDCState.SectorOverRead = true;
	}

	if (ValidateSector(CommandStatus.CurrentSectorPtr, CommandStatus.TrackAddr, CommandStatus.SectorLength))
	{
		CommandStatus.ByteWithinSector = 0;

		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY;
		UpdateNMIStatus();

		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
	else
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
	}
}

/*--------------------------------------------------------------------------*/

static void ReadInterrupt()
{
	bool LastByte = false;

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	if (CommandStatus.SectorsToGo < 0)
	{
		FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
		UpdateNMIStatus();
		return;
	}

	FDCState.DataReg = CommandStatus.CurrentSectorPtr->Data[CommandStatus.ByteWithinSector++];

	#if ENABLE_LOG
	WriteLog("ReadInterrupt called - DataReg=0x%02X ByteWithinSector=%d\n", DataReg, CommandStatus.ByteWithinSector);
	#endif

	// FSD - use the error result from the FSD file
	FDCState.ResultReg = CommandStatus.CurrentSectorPtr->Error;

	// If track has no error, but the "real" size has not been read
	if (CommandStatus.CurrentSectorPtr->Error == RESULT_REG_SUCCESS &&
	    CommandStatus.CurrentSectorPtr->RealSectorSize != CommandStatus.SectorLength)
	{
		FDCState.ResultReg = RESULT_REG_DATA_CRC_ERROR;
	}

	if (FDCState.SectorOverRead)
	{
		if (CommandStatus.CurrentSectorPtr->Error == RESULT_REG_SUCCESS)
		{
			FDCState.ResultReg = RESULT_REG_DATA_CRC_ERROR;
		}
		else if (CommandStatus.CurrentSectorPtr->Error == RESULT_REG_DELETED_DATA_FOUND)
		{
			FDCState.ResultReg = RESULT_REG_DELETED_DATA_CRC_ERROR;
		}
		else if (CommandStatus.CurrentSectorPtr->Error == RESULT_REG_DELETED_DATA_CRC_ERROR)
		{
			FDCState.ResultReg = RESULT_REG_DELETED_DATA_CRC_ERROR;
		}
	}

	// Same as above, but for deleted data
	if (CommandStatus.CurrentSectorPtr->Error == RESULT_REG_DELETED_DATA_FOUND &&
	    CommandStatus.CurrentSectorPtr->RealSectorSize != CommandStatus.SectorLength)
	{
		FDCState.ResultReg = RESULT_REG_DELETED_DATA_CRC_ERROR;
	}

	if (CommandStatus.CurrentSectorPtr->Error == RESULT_REG_DELETED_DATA_CRC_ERROR &&
	    GetFSDSectorSize(CommandStatus.CurrentSectorPtr->IDField.SectorLength) == CommandStatus.SectorLength &&
	    !FDCState.SectorOverRead)
	{
		FDCState.ResultReg = RESULT_REG_DELETED_DATA_FOUND;
	}

	// If track has deliberate error, but the id field sector size has been read.
	if (CommandStatus.CurrentSectorPtr->Error == 0xE0)
	{
		FDCState.ResultReg = CommandStatus.SectorLength == 128 ?
		                     RESULT_REG_SUCCESS : RESULT_REG_DATA_CRC_ERROR;
	}
	else if (CommandStatus.CurrentSectorPtr->Error == 0xE1)
	{
		FDCState.ResultReg = CommandStatus.SectorLength == 256 ?
		                     RESULT_REG_SUCCESS : RESULT_REG_DATA_CRC_ERROR;
	}
	else if (CommandStatus.CurrentSectorPtr->Error == 0xE2)
	{
		FDCState.ResultReg = CommandStatus.SectorLength == 512 ?
		                     RESULT_REG_SUCCESS : RESULT_REG_DATA_CRC_ERROR;
	}

	if (CommandStatus.CurrentSectorPtr->Error == RESULT_REG_DATA_CRC_ERROR &&
	    CommandStatus.CurrentSectorPtr->RealSectorSize == GetFSDSectorSize(CommandStatus.CurrentSectorPtr->IDField.SectorLength))
	{
		FDCState.ResultReg = RESULT_REG_DATA_CRC_ERROR;

		if (CommandStatus.ByteWithinSector % 5 == 0)
		{
			FDCState.DataReg >>= rand() % 8;
		}
	}

	if (CommandStatus.ByteWithinSector >= CommandStatus.SectorLength)
	{
		CommandStatus.ByteWithinSector = 0;

		// I don't know if this can cause the thing to step - I presume not for the moment
		if (--CommandStatus.SectorsToGo > 0)
		{
			CommandStatus.CurrentSector++;
			CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr,
			                                              CommandStatus.CurrentSector,
			                                              false);

			if (CommandStatus.CurrentSectorPtr == nullptr)
			{
				DoErr(RESULT_REG_SECTOR_NOT_FOUND);
				return;
			}
		}
		else
		{
			// Last sector done
			LastByte = true;
			CommandStatus.SectorsToGo = -1; // To let us bail out

			FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
			                     STATUS_REG_RESULT_FULL |
			                     STATUS_REG_INTERRUPT_REQUEST |
			                     STATUS_REG_NON_DMA_MODE;
			UpdateNMIStatus();

			SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger); // To pick up result
		}
	}

	if (!LastByte)
	{
		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
		                     STATUS_REG_INTERRUPT_REQUEST |
		                     STATUS_REG_NON_DMA_MODE;
		UpdateNMIStatus();

		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
}

/*--------------------------------------------------------------------------*/

// 128 Byte Single Record Read Data and Deleted Data Command
//
// Parameters:
// 0: Track Address (0-255)
// 1: Sector (0-255)

static void Do128ByteSR_ReadDataAndDeldCommand()
{
	DoSelects();
	DoLoadHead();

	#ifdef DEBUG_8271
	DebugTrace("8271: Read Data and Deleted Data (Single Record), Track %d Sector %d\n",
	           FDCState.Params[0],
	           FDCState.Params[1]);
	#endif

	const int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	// FSD - if special register is NOT being used to point to logical track
	if (!FDCState.UsingSpecial)
	{
		FDCState.FSDPhysicalTrack[Drive] = FDCState.Params[0];
	}

	FDCState.CurrentTrack[Drive] = FDCState.Params[0];

	// FSD - if internal track =0, seek track 0 too
	if (FDCState.CurrentTrack[Drive] == 0)
	{
		FDCState.FSDPhysicalTrack[Drive] = 0;
	}

	CommandStatus.CurrentTrackPtr = GetTrackPtr(FDCState.Params[0]);

	if (CommandStatus.CurrentTrackPtr == nullptr)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	// FSD - if track contains no data
	if (!CommandStatus.CurrentTrackPtr->TrackIsReadable)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr, FDCState.Params[1], false);

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	CommandStatus.TrackAddr     = FDCState.Params[0];
	CommandStatus.CurrentSector = FDCState.Params[1];
	CommandStatus.SectorsToGo   = 1;
	CommandStatus.SectorLength  = 128;

	if (ValidateSector(CommandStatus.CurrentSectorPtr, CommandStatus.TrackAddr, CommandStatus.SectorLength))
	{
		CommandStatus.ByteWithinSector = 0;

		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY;
		UpdateNMIStatus();

		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
	else
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
	}
}

/*--------------------------------------------------------------------------*/

static void Read128Interrupt()
{
	int LastByte = 0;

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	if (CommandStatus.SectorsToGo < 0)
	{
		FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
		UpdateNMIStatus();
		return;
	}

	FDCState.DataReg = CommandStatus.CurrentSectorPtr->Data[CommandStatus.ByteWithinSector++];

	FDCState.ResultReg = CommandStatus.CurrentSectorPtr->Error; // FSD - used to be 0

	// if error is just deleted data, then result = 0
	// if (ResultReg==0x20) {ResultReg=0;}

	// If track has no error, but the "real" size has not been read
	if (CommandStatus.CurrentSectorPtr->Error == 0 &&
	    CommandStatus.CurrentSectorPtr->RealSectorSize != CommandStatus.SectorLength)
	{
		FDCState.ResultReg = RESULT_REG_DATA_CRC_ERROR;
	}

	if (FDCState.SectorOverRead)
	{
		FDCState.ResultReg = RESULT_REG_DATA_CRC_ERROR;
	}

	// Same as above, but for deleted data
	if (CommandStatus.CurrentSectorPtr->Error == 0x20 &&
	    CommandStatus.CurrentSectorPtr->RealSectorSize != CommandStatus.SectorLength)
	{
		FDCState.ResultReg = RESULT_REG_DELETED_DATA_CRC_ERROR;
	}

	// If track has deliberate error, but the id field sector size has been read
	if (CommandStatus.CurrentSectorPtr->Error == 0xE1 &&
	    CommandStatus.SectorLength != 0x100)
	{
		FDCState.ResultReg = RESULT_REG_DATA_CRC_ERROR;
	}
	else if (CommandStatus.CurrentSectorPtr->Error == 0xE1 &&
	         CommandStatus.SectorLength == 0x100)
	{
		FDCState.ResultReg = RESULT_REG_SUCCESS;
	}

	if (CommandStatus.ByteWithinSector >= CommandStatus.SectorLength)
	{
		CommandStatus.ByteWithinSector = 0;

		// I don't know if this can cause the thing to step - I presume not for the moment
		if (--CommandStatus.SectorsToGo > 0)
		{
			CommandStatus.CurrentSector++;
			CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr, CommandStatus.CurrentSector, false);

			if (CommandStatus.CurrentSectorPtr == nullptr)
			{
				DoErr(RESULT_REG_SECTOR_NOT_FOUND);
				return;
			}
		}
		else
		{
			// Last sector done
			LastByte = 1;
			CommandStatus.SectorsToGo = -1; // To let us bail out

			FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
			                     STATUS_REG_RESULT_FULL |
			                     STATUS_REG_INTERRUPT_REQUEST |
			                     STATUS_REG_NON_DMA_MODE;
			UpdateNMIStatus();

			SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger); // To pick up result
		}
	}

	if (!LastByte)
	{
		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
		                     STATUS_REG_INTERRUPT_REQUEST |
		                     STATUS_REG_NON_DMA_MODE;
		UpdateNMIStatus();

		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
}

/*--------------------------------------------------------------------------*/

// Read Data and Deleted Data Command
//
// Parameters:
// 0: Logical Track Address (0-255)
// 1: Logical Sector Address (0-255)
// 2: Sector Size / Number of Sectors
//    (bit 7-bit 5) determine the length of the disk record
//    0 0 0  128 bytes
//    0 0 1  256 bytes
//    0 1 0  512 bytes
//    0 1 1  1024 bytes
//    1 0 0  2048 bytes
//    1 0 1  4096 bytes
//    1 1 0  8192 bytes
//    1 1 1  16384 bytes

static void DoVarLength_ReadDataAndDeldCommand()
{
	// Use normal read command for now - deleted data not supported
	DoVarLength_ReadDataCommand();
}

/*--------------------------------------------------------------------------*/

// The Read ID command transfers the specified number of ID fields Into
// memory (beginning with the first ID field after Index). The CRC character
// is checked but not transferred. These fields are entered into memory in the
// order in which they are physically located on the disk, with the first
// field being the one starting at the index pulse.
//
// The ID field is seven bytes long and is written for each sector when the
// track is formatted. Each ID field consists of:
//
// * an ID field Address Mark
// * a Cylinder Number byte which identifies the track number
// * a Head Number byte which specifies the head used (top or bottom) to access
//   the sector
// * a Record Number byte identifying the sector number (1 through 26 for
//   128 byte sectors)
// * an N-byte specifying the byte length of the sector
// * two CRC (Cyclic Redundancy Check) bytes
//
// Parameters:
// 0: Track Address
// 1: Zero
// 2: Number of ID Fields

static void DoReadIDCommand()
{
	DoSelects();
	DoLoadHead();

	#ifdef DEBUG_8271
	DebugTrace("8271: Read ID, Track %d Number of ID Fields %d\n",
	           FDCState.Params[0],
	           FDCState.Params[2]);
	#endif

	const int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	// Internal_CurrentTrack[Drive]=Params[0];
	FDCState.FSDPhysicalTrack[Drive] = FDCState.Params[0];
	CommandStatus.CurrentTrackPtr = GetTrackPtrPhysical(FDCState.FSDPhysicalTrack[Drive]);

	if (CommandStatus.CurrentTrackPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	// FSD - was GetSectorPtr
	CommandStatus.CurrentSectorPtr = GetSectorPtrForTrackID(CommandStatus.CurrentTrackPtr, 0, false);

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	CommandStatus.TrackAddr     = FDCState.Params[0];
	CommandStatus.CurrentSector = 0;
	CommandStatus.SectorsToGo   = FDCState.Params[2];

	if (CommandStatus.SectorsToGo == 0)
	{
		CommandStatus.SectorsToGo = 0x20;
	}

	CommandStatus.ByteWithinSector = 0;
	SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	FDCState.StatusReg = STATUS_REG_COMMAND_BUSY;
	UpdateNMIStatus();

	// FSDPhysicalTrack = FSDPhysicalTrack + 1;
}

/*--------------------------------------------------------------------------*/

static void ReadIDInterrupt()
{
	bool LastByte = false;

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	if (CommandStatus.SectorsToGo < 0)
	{
		FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
		UpdateNMIStatus();
		return;
	}

	if (CommandStatus.ByteWithinSector == 0)
	{
		FDCState.DataReg = CommandStatus.CurrentSectorPtr->IDField.LogicalTrack;
	}
	else if (CommandStatus.ByteWithinSector == 1)
	{
		FDCState.DataReg = CommandStatus.CurrentSectorPtr->IDField.HeadNum;
	}
	else if (CommandStatus.ByteWithinSector == 2)
	{
		FDCState.DataReg = CommandStatus.CurrentSectorPtr->IDField.LogicalSector; // RecordNum
	}
	else if (CommandStatus.ByteWithinSector == 3)
	{
		FDCState.DataReg = CommandStatus.CurrentSectorPtr->IDField.SectorLength;
	}

	CommandStatus.ByteWithinSector++;

	FDCState.ResultReg = RESULT_REG_SUCCESS;

	if (CommandStatus.ByteWithinSector >= 4)
	{
		CommandStatus.ByteWithinSector = 0;

		if (--CommandStatus.SectorsToGo > 0)
		{
			if (++CommandStatus.CurrentSector == CommandStatus.CurrentTrackPtr->NSectors)
			{
				CommandStatus.CurrentSector = 0;
			}

			const int Drive = GetSelectedDrive();

			FDCState.PositionInTrack[Drive] = CommandStatus.CurrentSector; // FSD

			CommandStatus.CurrentSectorPtr = GetSectorPtrForTrackID(CommandStatus.CurrentTrackPtr,
			                                                        CommandStatus.CurrentSector,
			                                                        false);

			if (CommandStatus.CurrentSectorPtr == nullptr)
			{
				DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
				return;
			}
		}
		else
		{
			// Last sector done
			LastByte = true;
			// PositionInTrack = 0; // FSD - track position to zero
			CommandStatus.SectorsToGo = -1; // To let us bail out

			FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
			                     STATUS_REG_INTERRUPT_REQUEST |
			                     STATUS_REG_NON_DMA_MODE;
			UpdateNMIStatus();

			SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger); // To pick up result
		}
	}

	if (!LastByte)
	{
		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
		                     STATUS_REG_INTERRUPT_REQUEST |
		                     STATUS_REG_NON_DMA_MODE;
		UpdateNMIStatus();
		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
}

/*--------------------------------------------------------------------------*/

// 128 Byte Single Record Verify Data and Deleted Data Command
//
// Parameters:
// 0: Track Address (0-255)
// 1: Sector (0-255)

static void Do128ByteSR_VerifyDataAndDeldCommand()
{
	DoSelects();
	NotImp("Do128ByteSR_VerifyDataAndDeldCommand");
}

/*--------------------------------------------------------------------------*/

// The Verify Data and Deleted Data command is identical to the Read Data and
// Deleted Data command except that the data is not transferred to memory.
// This command is used to check that a record or a group of records has
// been written correctly by verifying the CRC character.
//
// Parameters:
// 0: Logical Track Address (0-255)
// 1: Logical Sector Address (0-255)
// 2: Sector Size / Number of Sectors
//    (bit 7-bit 5) determine the length of the disk record
//    0 0 0  128 bytes
//    0 0 1  256 bytes
//    0 1 0  512 bytes
//    0 1 1  1024 bytes
//    1 0 0  2048 bytes
//    1 0 1  4096 bytes
//    1 1 0  8192 bytes
//    1 1 1  16384 bytes

static void DoVarLength_VerifyDataAndDeldCommand()
{
	DoSelects();

	#ifdef DEBUG_8271
	DebugTrace("8271: Verify Data and Deleted Data (Variable Length), Track %d Sector %d, Sectors %d, SectorLength %d\n",
	           FDCState.Params[0],
	           FDCState.Params[1],
	           FDCState.Params[2] & 0x1F,
	           1 << (7 + ((FDCState.Params[2] >> 5) & 7)));
	#endif

	const int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	FDCState.CurrentTrack[Drive] = FDCState.Params[0];
	FDCState.FSDPhysicalTrack[Drive] = FDCState.Params[0];
	FDCState.FSDLogicalTrack[Drive] = FDCState.Params[0];
	CommandStatus.CurrentTrackPtr = GetTrackPtr(FDCState.FSDLogicalTrack[Drive]);

	if (CommandStatus.CurrentTrackPtr == nullptr)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr, FDCState.Params[1], false);

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
		return;
	}

	FDCState.ResultReg = CommandStatus.CurrentSectorPtr->Error;

	if (FDCState.ResultReg != 0)
	{
		FDCState.StatusReg = FDCState.ResultReg;
	}
	else
	{
		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY;
	}

	UpdateNMIStatus();
	SetTrigger(100, Disc8271Trigger); // A short delay to causing an interrupt
}

/*--------------------------------------------------------------------------*/

static void VerifyInterrupt()
{
	FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
	UpdateNMIStatus();
	FDCState.ResultReg = RESULT_REG_SUCCESS; // All OK
}

/*--------------------------------------------------------------------------*/

// Format Command
//
// Parameters:
// 0: Track Address (0-255)
// 1: Gap 3 Size minus 6
// 2: Record Length / Number of Sectors per Track
// 3: Gap 5 Size minus 6
// 4: Gap 1 Size minus 6

static void DoFormatCommand()
{
	DoSelects();
	DoLoadHead();

	const int Drive = GetSelectedDrive();

	if (Drive < 0)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	if (!DiscStatus[Drive].Writeable)
	{
		DoErr(RESULT_REG_WRITE_PROTECT);
		return;
	}

	FDCState.CurrentTrack[Drive] = FDCState.Params[0];
	CommandStatus.CurrentTrackPtr = GetTrackPtr(FDCState.Params[0]);

	if (CommandStatus.CurrentTrackPtr == nullptr)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr, 0, false);

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
		return;
	}

	CommandStatus.TrackAddr     = FDCState.Params[0];
	CommandStatus.CurrentSector = 0;
	CommandStatus.SectorsToGo   = FDCState.Params[2] & 0x1F;
	CommandStatus.SectorLength  = 1 << (7 + ((FDCState.Params[2] >> 5) & 7));

	if (CommandStatus.SectorsToGo == 10 && CommandStatus.SectorLength == 256)
	{
		CommandStatus.ByteWithinSector = 0;
		CommandStatus.FirstWriteInt = true;

		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY;
		UpdateNMIStatus();

		SetTrigger(TIME_BETWEEN_BYTES, Disc8271Trigger);
	}
	else
	{
		DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
	}
}

/*--------------------------------------------------------------------------*/

static void FormatInterrupt()
{
	bool LastByte = false;

	if (CommandStatus.CurrentSectorPtr == nullptr)
	{
		DoErr(RESULT_REG_SECTOR_NOT_FOUND);
		return;
	}

	if (CommandStatus.SectorsToGo < 0)
	{
		FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
		UpdateNMIStatus();
		return;
	}

	if (!CommandStatus.FirstWriteInt)
	{
		// Ignore the ID data for now - just count the bytes
		CommandStatus.ByteWithinSector++;
	}
	else
	{
		CommandStatus.FirstWriteInt = false;
	}

	FDCState.ResultReg = RESULT_REG_SUCCESS;

	if (CommandStatus.ByteWithinSector >= 4)
	{
		// Fill sector with 0xe5 chars
		for (int i = 0; i < 256; ++i)
		{
			CommandStatus.CurrentSectorPtr->Data[i] = (unsigned char)0xe5;
		}

		CommandStatus.ByteWithinSector = 0;

		if (--CommandStatus.SectorsToGo > 0)
		{
			CommandStatus.CurrentSector++;
			CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr,
			                                              CommandStatus.CurrentSector,
			                                              false);

			if (CommandStatus.CurrentSectorPtr == nullptr)
			{
				DoErr(RESULT_REG_DRIVE_NOT_PRESENT); // Sector not found
				return;
			}
		}
		else
		{
			// Last sector done, write the track back to disc
			if (SaveTrackImage(FDCState.Select[0] ? 0 : 1, CURRENT_HEAD, CommandStatus.TrackAddr))
			{
				LastByte = true;
				CommandStatus.SectorsToGo = -1; // To let us bail out

				FDCState.StatusReg = STATUS_REG_RESULT_FULL;
				UpdateNMIStatus();

				SetTrigger(0, Disc8271Trigger); // To pick up result
			}
			else
			{
				DoErr(RESULT_REG_WRITE_PROTECT);
			}
		}
	}

	if (!LastByte)
	{
		FDCState.StatusReg = STATUS_REG_COMMAND_BUSY |
		                     STATUS_REG_INTERRUPT_REQUEST |
		                     STATUS_REG_NON_DMA_MODE;
		UpdateNMIStatus();

		SetTrigger(TIME_BETWEEN_BYTES * 256, Disc8271Trigger);
	}
}

/*--------------------------------------------------------------------------*/

static void DoSeekCommand()
{
	DoSelects();
	DoLoadHead();

	#ifdef DEBUG_8271
	DebugTrace("8271: Seek, Track %d\n",
	           FDCState.Params[0]);
	#endif

	int Drive = GetSelectedDrive();

	if (Drive<0)
	{
		DoErr(RESULT_REG_DRIVE_NOT_READY);
		return;
	}

	FDCState.DRDSC = 0;
	FDCState.CurrentTrack[Drive] = FDCState.Params[0];
	FDCState.FSDPhysicalTrack[Drive] = FDCState.Params[0]; // FSD - where to start seeking data store
	FDCState.UsingSpecial = false;
	FDCState.PositionInTrack[Drive] = 0;

	FDCState.StatusReg = STATUS_REG_COMMAND_BUSY;
	UpdateNMIStatus();

	SetTrigger(100, Disc8271Trigger); // A short delay to causing an interrupt
}

/*--------------------------------------------------------------------------*/

static void SeekInterrupt()
{
	FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
	UpdateNMIStatus();
	FDCState.ResultReg = RESULT_REG_SUCCESS; // All OK
}

/*--------------------------------------------------------------------------*/

static void DoReadDriveStatusCommand()
{
	bool Track0 = false;
	bool WriteProt = false;

	if (FDCState.Command & 0x40)
	{
		Track0 = FDCState.CurrentTrack[0] == 0;
		WriteProt = !DiscStatus[0].Writeable;
	}

	if (FDCState.Command & 0x80)
	{
		Track0 = FDCState.CurrentTrack[1] == 0;
		WriteProt = !DiscStatus[1].Writeable;
	}

	FDCState.DRDSC++;

	FDCState.ResultReg = 0x80 | (FDCState.Select[1] ? 0x40 : 0)
	                          | (FDCState.Select[0] ? 0x04 : 0)
	                          | (WriteProt  ? 0x08 : 0)
	                          | (Track0     ? 0x02 : 0);

	#ifdef DEBUG_8271
	DebugTrace("8271: Read Drive Status, Result: %02X (Select 0: %d, Select 1: %d, WriteProt: %d, Track0: %d)\n",
	           FDCState.ResultReg,
	           FDCState.Select[0],
	           FDCState.Select[1],
	           WriteProt,
	           Track0);
	#endif

	FDCState.StatusReg |= STATUS_REG_RESULT_FULL;
	UpdateNMIStatus();
}

/*--------------------------------------------------------------------------*/

// See Intel 8271 data sheet, page 15, ADUG page 39-40

static void DoSpecifyCommand()
{
	switch (FDCState.Params[0])
	{
		case 0x0D: // Initialisation
			FDCState.StepRate = FDCState.Params[1];
			FDCState.HeadSettlingTime = FDCState.Params[2];
			FDCState.IndexCountBeforeHeadUnload = (FDCState.Params[3] & 0xf0) >> 4;
			FDCState.HeadLoadTime = FDCState.Params[3] & 0x0f;

			#ifdef DEBUG_8271
			DebugTrace("8271: Specify, Type %02X (Initialisation), StepRate %d, HeadSettlingTime %d, IndexCountBeforeHeadUnload %d, HeadLoadTime %d\n",
			           FDCState.Params[0],
			           FDCState.StepRate,
			           FDCState.HeadSettlingTime,
			           FDCState.IndexCountBeforeHeadUnload,
			           FDCState.HeadLoadTime);
			#endif
			break;

		case 0x10: // Load bad tracks, surface 0
			FDCState.BadTracks[0][0] = FDCState.Params[1];
			FDCState.BadTracks[0][1] = FDCState.Params[2];
			FDCState.CurrentTrack[0] = FDCState.Params[3];

			#ifdef DEBUG_8271
			DebugTrace("8271: Specify, Type %02X (Load Bad Tracks, Surface 0), Bad Track 0: %d, Bad Track 1: %d, Current Track: %d\n",
			           FDCState.Params[0],
			           FDCState.BadTracks[0][0],
			           FDCState.BadTracks[0][1],
			           FDCState.CurrentTrack[0]);
			#endif
			break;

		case 0x18: // Load bad tracks, surface 1
			FDCState.BadTracks[1][0] = FDCState.Params[1];
			FDCState.BadTracks[1][1] = FDCState.Params[2];
			FDCState.CurrentTrack[1] = FDCState.Params[3];

			#ifdef DEBUG_8271
			DebugTrace("8271: Specify, Type %02X (Load Bad Tracks, Surface 1), Bad Track 0: %d, Bad Track 1: %d, Current Track: %d\n",
			           FDCState.Params[0],
			           FDCState.BadTracks[0][0],
			           FDCState.BadTracks[0][1],
			           FDCState.CurrentTrack[0]);
			#endif
			break;
	}
}

/*--------------------------------------------------------------------------*/

// Write Special Register Command
//
// Parameters:
// 0: Register Address
// 1: Data

static void DoWriteSpecialCommand()
{
	#ifdef DEBUG_8271
	DebugTrace("8271: Write Special Register, Register %02X, Data %02X\n",
	           FDCState.Params[0],
	           FDCState.Params[1]);
	#endif

	DoSelects();

	switch (FDCState.Params[0])
	{
		case SPECIAL_REG_SCAN_SECTOR_NUMBER:
			FDCState.ScanSectorNum = FDCState.Params[1];
			break;

		case SPECIAL_REG_SCAN_COUNT_MSB:
			FDCState.ScanCount &= 0xff;
			FDCState.ScanCount |= FDCState.Params[1] << 8;
			break;

		case SPECIAL_REG_SCAN_COUNT_LSB:
			FDCState.ScanCount &= 0xff00;
			FDCState.ScanCount |= FDCState.Params[1];
			break;

		case SPECIAL_REG_SURFACE_0_CURRENT_TRACK:
			FDCState.CurrentTrack[0] = FDCState.Params[1];
			FDCState.FSDLogicalTrack[0] = FDCState.Params[1];
			// FSD - using special register, so different track from seek
			FDCState.UsingSpecial = FDCState.Params[1] != FDCState.FSDPhysicalTrack[0];
			FDCState.DRDSC = 0;
			break;

		case SPECIAL_REG_SURFACE_1_CURRENT_TRACK:
			FDCState.CurrentTrack[1] = FDCState.Params[1];
			break;

		case SPECIAL_REG_MODE_REGISTER:
			FDCState.ModeReg = FDCState.Params[1];
			break;

		case SPECIAL_REG_DRIVE_CONTROL_OUTPUT_PORT:
			FDCState.DriveControlOutputPort = FDCState.Params[1];
			FDCState.Select[0] = (FDCState.Params[1] & 0x40) != 0;
			FDCState.Select[1] = (FDCState.Params[1] & 0x80) != 0;
			break;

		case SPECIAL_REG_DRIVE_CONTROL_INPUT_PORT:
			FDCState.DriveControlInputPort = FDCState.Params[1];
			break;

		case SPECIAL_REG_SURFACE_0_BAD_TRACK_1:
			FDCState.BadTracks[0][0] = FDCState.Params[1];
			break;

		case SPECIAL_REG_SURFACE_0_BAD_TRACK_2:
			FDCState.BadTracks[0][1] = FDCState.Params[1];
			break;

		case SPECIAL_REG_SURFACE_1_BAD_TRACK_1:
			FDCState.BadTracks[1][0] = FDCState.Params[1];
			break;

		case SPECIAL_REG_SURFACE_1_BAD_TRACK_2:
			FDCState.BadTracks[1][1] = FDCState.Params[1];
			break;

		default:
			#if ENABLE_LOG
			WriteLog("Write to bad special register\n");
			#endif
			break;
	}
}

/*--------------------------------------------------------------------------*/

// Read Special Register Command
//
// Parameters:
// 0: Register Address

static void DoReadSpecialCommand()
{
	#ifdef DEBUG_8271
	DebugTrace("8271: Read Special Register, Register %02X\n",
	           FDCState.Params[0]);
	#endif

	DoSelects();

	switch (FDCState.Params[0])
	{
		case SPECIAL_REG_SCAN_SECTOR_NUMBER:
			FDCState.ResultReg = FDCState.ScanSectorNum;
			break;

		case SPECIAL_REG_SCAN_COUNT_MSB:
			FDCState.ResultReg = (FDCState.ScanCount >> 8) & 0xff;
			break;

		case SPECIAL_REG_SCAN_COUNT_LSB:
			FDCState.ResultReg = FDCState.ScanCount & 0xff;
			break;

		case SPECIAL_REG_SURFACE_0_CURRENT_TRACK:
			FDCState.ResultReg = FDCState.CurrentTrack[0];
			break;

		case SPECIAL_REG_SURFACE_1_CURRENT_TRACK:
			FDCState.ResultReg = FDCState.CurrentTrack[1];
			break;

		case SPECIAL_REG_MODE_REGISTER:
			FDCState.ResultReg = FDCState.ModeReg;
			break;

		case SPECIAL_REG_DRIVE_CONTROL_OUTPUT_PORT:
			FDCState.ResultReg = FDCState.DriveControlOutputPort;
			break;

		case SPECIAL_REG_DRIVE_CONTROL_INPUT_PORT:
			FDCState.ResultReg = FDCState.DriveControlInputPort;
			break;

		case SPECIAL_REG_SURFACE_0_BAD_TRACK_1:
			FDCState.ResultReg = FDCState.BadTracks[0][0];
			break;

		case SPECIAL_REG_SURFACE_0_BAD_TRACK_2:
			FDCState.ResultReg = FDCState.BadTracks[0][1];
			break;

		case SPECIAL_REG_SURFACE_1_BAD_TRACK_1:
			FDCState.ResultReg = FDCState.BadTracks[1][0];
			break;

		case SPECIAL_REG_SURFACE_1_BAD_TRACK_2:
			FDCState.ResultReg = FDCState.BadTracks[1][1];
			break;

		default:
			#if ENABLE_LOG
			WriteLog("Read of bad special register\n");
			#endif
			return;
	}

	FDCState.StatusReg |= STATUS_REG_RESULT_FULL;
	UpdateNMIStatus();
}

/*--------------------------------------------------------------------------*/

static void DoBadCommand()
{
}

/*--------------------------------------------------------------------------*/

// The following table is used to parse commands from the command number
// written into the command register - it can't distinguish between subcommands
// selected from the first parameter.

typedef void (*CommandFunc)();

struct PrimaryCommandLookupType {
	unsigned char CommandNum;
	unsigned char Mask; // Mask command with this before comparing with CommandNum - allows drive ID to be removed
	int NParams; // Number of parameters to follow
	CommandFunc ToCall; // Called after all paameters have arrived
	CommandFunc IntHandler; // Called when interrupt requested by command is about to happen
	const char *Ident; // Mainly for debugging
};

static const PrimaryCommandLookupType PrimaryCommandLookup[] = {
	{ 0x00, 0x3f, 3, DoVarLength_ScanDataCommand,          nullptr,          "Scan Data (Variable Length/Multi-Record)" },
	{ 0x04, 0x3f, 3, DoVarLength_ScanDataAndDeldCommand,   nullptr,          "Scan Data & deleted data (Variable Length/Multi-Record)" },
	{ 0x0a, 0x3f, 2, Do128ByteSR_WriteDataCommand,         nullptr,          "Write Data (128 byte/single record)" },
	{ 0x0b, 0x3f, 3, DoVarLength_WriteDataCommand,         WriteInterrupt,   "Write Data (Variable Length/Multi-Record)" },
	{ 0x0e, 0x3f, 2, Do128ByteSR_WriteDeletedDataCommand,  nullptr,          "Write Deleted Data (128 byte/single record)" },
	{ 0x0f, 0x3f, 3, DoVarLength_WriteDeletedDataCommand,  nullptr,          "Write Deleted Data (Variable Length/Multi-Record)" },
	{ 0x12, 0x3f, 2, Do128ByteSR_ReadDataCommand,          nullptr,          "Read Data (128 byte/single record)" },
	{ 0x13, 0x3f, 3, DoVarLength_ReadDataCommand,          ReadInterrupt,    "Read Data (Variable Length/Multi-Record)" },
	{ 0x16, 0x3f, 2, Do128ByteSR_ReadDataAndDeldCommand,   Read128Interrupt, "Read Data & deleted data (128 byte/single record)" },
	{ 0x17, 0x3f, 3, DoVarLength_ReadDataAndDeldCommand,   ReadInterrupt,    "Read Data & deleted data (Variable Length/Multi-Record)" },
	{ 0x1b, 0x3f, 3, DoReadIDCommand,                      ReadIDInterrupt,  "ReadID" },
	{ 0x1e, 0x3f, 2, Do128ByteSR_VerifyDataAndDeldCommand, nullptr,          "Verify Data and Deleted Data (128 byte/single record)" },
	{ 0x1f, 0x3f, 3, DoVarLength_VerifyDataAndDeldCommand, VerifyInterrupt,  "Verify Data and Deleted Data (Variable Length/Multi-Record)" },
	{ 0x23, 0x3f, 5, DoFormatCommand,                      FormatInterrupt,  "Format" },
	{ 0x29, 0x3f, 1, DoSeekCommand,                        SeekInterrupt,    "Seek" },
	{ 0x2c, 0x3f, 0, DoReadDriveStatusCommand,             nullptr,          "Read drive status" },
	{ 0x35, 0xff, 4, DoSpecifyCommand,                     nullptr,          "Specify" },
	{ 0x3a, 0x3f, 2, DoWriteSpecialCommand,                nullptr,          "Write special registers" },
	{ 0x3d, 0x3f, 1, DoReadSpecialCommand,                 nullptr,          "Read special registers" },
	{ 0,    0,    0, DoBadCommand,                         nullptr,          "Unknown command" } // Terminator due to 0 mask matching all
};

/*--------------------------------------------------------------------------*/

// returns a pointer to the data structure for the given command
// If no matching command is given, the pointer points to an entry with a 0
// mask, with a sensible function to call.

static const PrimaryCommandLookupType *CommandPtrFromNumber(int CommandNumber)
{
	const PrimaryCommandLookupType *presptr = PrimaryCommandLookup;

	while (presptr->CommandNum != (presptr->Mask & CommandNumber))
	{
		presptr++;
	}

	return presptr;

	// FSD - could FSDPhysicalTrack = -1 here?
}

/*--------------------------------------------------------------------------*/

// Address is in the range 0-7 - with the fe80 etc stripped out

unsigned char Disc8271Read(int Address)
{
	unsigned char Value = 0;

	if (!Disc8271Enabled)
	{
		return 0xFF;
	}

	switch (Address)
	{
		case 0:
			#if ENABLE_LOG
			WriteLog("8271 Status register read (0x%0X)\n", StatusReg);
			#endif

			Value = FDCState.StatusReg;
			break;

		case 1:
			#if ENABLE_LOG
			WriteLog("8271 Result register read (0x%02X)\n", ResultReg);
			#endif

			// Clear interrupt request and result reg full flag
			FDCState.StatusReg &= ~(STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST);
			UpdateNMIStatus();
			Value = FDCState.ResultReg;
			FDCState.ResultReg = RESULT_REG_SUCCESS; // Register goes to 0 after its read
			break;

		case 4:
			#if ENABLE_LOG
			WriteLog("8271 data register read\n");
			#endif

			// Clear interrupt and non-dma request - not stated but DFS never looks at result reg!
			FDCState.StatusReg &= ~(STATUS_REG_INTERRUPT_REQUEST | STATUS_REG_NON_DMA_MODE);
			UpdateNMIStatus();
			Value = FDCState.DataReg;
			break;

		default:
			#if ENABLE_LOG
			WriteLog("8271: Read to unknown register address=%04X\n", Address);
			#endif
			break;
	}

	return Value;
}

/*--------------------------------------------------------------------------*/

static void CommandRegWrite(unsigned char Value)
{
	const PrimaryCommandLookupType *ptr = CommandPtrFromNumber(Value);

	#if ENABLE_LOG
	WriteLog("8271: Command register write value=0x%02X (Name=%s)\n", Value, ptr->Ident);
	#endif

	FDCState.Command = Value;
	FDCState.CommandParamCount = ptr->NParams;
	FDCState.CurrentParam = 0;

	FDCState.StatusReg |= STATUS_REG_COMMAND_BUSY | STATUS_REG_RESULT_FULL; // Observed on beeb for read special
	UpdateNMIStatus();

	// No parameters then call routine immediately
	if (FDCState.CommandParamCount == 0)
	{
		FDCState.StatusReg &= 0x7e;
		UpdateNMIStatus();
		ptr->ToCall();
	}
}

/*--------------------------------------------------------------------------*/

static void ParamRegWrite(unsigned char Value)
{
	// Parameter wanted?
	if (FDCState.CurrentParam >= FDCState.CommandParamCount)
	{
		#if ENABLE_LOG
		WriteLog("8271: Unwanted parameter register write value=0x%02X\n", Value);
		#endif
	}
	else
	{
		FDCState.Params[FDCState.CurrentParam++] = Value;

		FDCState.StatusReg &= 0xfe; // Observed on beeb
		UpdateNMIStatus();

		// Got all params yet?
		if (FDCState.CurrentParam >= FDCState.CommandParamCount)
		{
			FDCState.StatusReg &= 0x7e; // Observed on beeb
			UpdateNMIStatus();

			const PrimaryCommandLookupType *ptr = CommandPtrFromNumber(FDCState.Command);

			#if ENABLE_LOG
			WriteLog("<Disc access> 8271: All parameters arrived for '%s':", ptr->Ident);

			for (int i = 0; i < PresentParam; i++)
			{
				WriteLog(" %02X", Params[i]);
			}

			WriteLog("\n");
			#endif

			ptr->ToCall();
		}
	}
}

/*--------------------------------------------------------------------------*/

// Address is in the range 0-7 - with the fe80 etc stripped out

void Disc8271Write(int Address, unsigned char Value)
{
	if (!Disc8271Enabled)
	{
		return;
	}

	// Clear a pending head unload
	if (FDCState.DriveHeadUnloadPending)
	{
		FDCState.DriveHeadUnloadPending = false;
		ClearTrigger(Disc8271Trigger);
	}

	switch (Address)
	{
		case 0:
			CommandRegWrite(Value);
			break;

		case 1:
			ParamRegWrite(Value);
			break;

		case 2:
			// The caller should write a 1 and then >11 cycles later a 0 - but I'm just going
			// to reset on both edges
			Disc8271Reset();
			break;

		case 4:
			FDCState.DataReg = Value;

			FDCState.StatusReg &= ~(STATUS_REG_INTERRUPT_REQUEST | STATUS_REG_NON_DMA_MODE);
			UpdateNMIStatus();
			break;

		default:
			break;
	}

	DriveHeadScheduleUnload();
}

/*--------------------------------------------------------------------------*/

static void DriveHeadScheduleUnload()
{
	// Schedule head unload when nothing else is pending.
	// This is mainly for the sound effects, but it also marks the drives as
	// not ready when the motor stops.
	if (FDCState.DriveHeadLoaded && Disc8271Trigger == CycleCountTMax)
	{
		SetTrigger(4000000, Disc8271Trigger); // 2s delay to unload
		FDCState.DriveHeadUnloadPending = true;
	}
}

/*--------------------------------------------------------------------------*/

bool DiscDriveSoundEnabled = false;

static bool DriveHeadMotorUpdate()
{
	// This is mainly for the sound effects, but it also marks the drives as
	// not ready when the motor stops.
	int Drive = 0;

	if (FDCState.DriveHeadUnloadPending)
	{
		// Mark drives as not ready
		FDCState.Select[0] = false;
		FDCState.Select[1] = false;
		FDCState.DriveHeadUnloadPending = false;
		if (FDCState.DriveHeadLoaded && DiscDriveSoundEnabled)
			PlaySoundSample(SAMPLE_HEAD_UNLOAD, false);
		FDCState.DriveHeadLoaded = false;
		StopSoundSample(SAMPLE_DRIVE_MOTOR);
		StopSoundSample(SAMPLE_HEAD_SEEK);

		LEDs.FloppyDisc[0] = false;
		LEDs.FloppyDisc[1] = false;
		return true;
	}

	if (!DiscDriveSoundEnabled)
	{
		FDCState.DriveHeadLoaded = true;
		return false;
	}

	if (!FDCState.DriveHeadLoaded)
	{
		if (FDCState.Select[0]) LEDs.FloppyDisc[0] = true;
		if (FDCState.Select[1]) LEDs.FloppyDisc[1] = true;

		PlaySoundSample(SAMPLE_DRIVE_MOTOR, true);
		FDCState.DriveHeadLoaded = true;
		PlaySoundSample(SAMPLE_HEAD_LOAD, false);
		SetTrigger(SAMPLE_HEAD_LOAD_CYCLES, Disc8271Trigger);
		return true;
	}

	if (FDCState.Select[0]) Drive = 0;
	if (FDCState.Select[1]) Drive = 1;

	StopSoundSample(SAMPLE_HEAD_SEEK);

	if (FDCState.DriveHeadPosition[Drive] != FDCState.FSDPhysicalTrack[Drive])
	{
		int Tracks = abs(FDCState.DriveHeadPosition[Drive] - FDCState.FSDPhysicalTrack[Drive]);

		if (Tracks > 1)
		{
			PlaySoundSample(SAMPLE_HEAD_SEEK, true);
			SetTrigger(Tracks * SAMPLE_HEAD_SEEK_CYCLES_PER_TRACK, Disc8271Trigger);
		}
		else
		{
			PlaySoundSample(SAMPLE_HEAD_STEP, false);
			SetTrigger(SAMPLE_HEAD_STEP_CYCLES, Disc8271Trigger);
		}

		if (FDCState.DriveHeadPosition[Drive] < FDCState.FSDPhysicalTrack[Drive])
		{
			FDCState.DriveHeadPosition[Drive] = (unsigned char)(FDCState.DriveHeadPosition[Drive] + Tracks);
		}
		else
		{
			FDCState.DriveHeadPosition[Drive] = (unsigned char)(FDCState.DriveHeadPosition[Drive] - Tracks);
		}

		return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

void Disc8271_poll_real()
{
	ClearTrigger(Disc8271Trigger);

	if (DriveHeadMotorUpdate())
	{
		return;
	}

	// Set the interrupt flag in the status register
	FDCState.StatusReg |= STATUS_REG_INTERRUPT_REQUEST;
	UpdateNMIStatus();

	if (CommandStatus.NextInterruptIsErr != RESULT_REG_DATA_CRC_ERROR &&
	    CommandStatus.NextInterruptIsErr != RESULT_REG_DELETED_DATA_FOUND &&
	    CommandStatus.NextInterruptIsErr != RESULT_REG_SUCCESS)
	{
		FDCState.ResultReg = CommandStatus.NextInterruptIsErr;

		FDCState.StatusReg = STATUS_REG_RESULT_FULL | STATUS_REG_INTERRUPT_REQUEST;
		UpdateNMIStatus();

		CommandStatus.NextInterruptIsErr = RESULT_REG_SUCCESS;
	}
	else
	{
		// Should only happen while a command is still active
		const PrimaryCommandLookupType *comptr = CommandPtrFromNumber(FDCState.Command);
		if (comptr->IntHandler != nullptr) comptr->IntHandler();
	}

	DriveHeadScheduleUnload();
}

/*--------------------------------------------------------------------------*/

void FreeDiscImage(int Drive)
{
	for (int Head = 0; Head < 2; Head++)
	{
		for (int Track = 0; Track < TRACKS_PER_DRIVE; Track++)
		{
			const int SectorsPerTrack = DiscStatus[Drive].Tracks[Head][Track].LogicalSectors;

			SectorType *SecPtr = DiscStatus[Drive].Tracks[Head][Track].Sectors;

			if (SecPtr != nullptr)
			{
				for (int Sector = 0; Sector < SectorsPerTrack; Sector++)
				{
					if (SecPtr[Sector].Data != nullptr)
					{
						free(SecPtr[Sector].Data);
						SecPtr[Sector].Data = nullptr;
					}
				}

				free(SecPtr);

				DiscStatus[Drive].Tracks[Head][Track].Sectors = nullptr;
			}
		}
	}

	CommandStatus.CurrentTrackPtr = nullptr;
	CommandStatus.CurrentSectorPtr = nullptr;
}

/*--------------------------------------------------------------------------*/

Disc8271Result LoadSimpleDiscImage(const char *FileName, int DriveNum, int HeadNum, int Tracks)
{
	std::ifstream Input(FileName, std::ios::in | std::ios::binary);

	if (!Input)
	{
		return Disc8271Result::Failed;
	}

	Disc8271Result Result = Disc8271Result::Success;

	// JGH, 26-Dec-2011
	DiscStatus[DriveNum].NumHeads = 1; // 1 = TRACKS_PER_DRIVE SSD image
	                                   // 2 = 2 * TRACKS_PER_DRIVE DSD or SSD image
	DiscInfo[DriveNum].DoubleSidedSSD = false;

	// Check for non-interleaved double-sided SSD image.
	// Example file: SmallC72.ssd from https://mdfs.net/Mirror/Archive/JGH/
	int Heads = 1;

	Input.seekg(0, std::ios::end);

	if (Input.tellg() > 80 * DFS_SECTORS_PER_TRACK * DFS_SECTOR_SIZE)
	{
		Heads = 2; // Long sequential image continues onto side 1
		DiscStatus[DriveNum].NumHeads = 2;
		DiscInfo[DriveNum].DoubleSidedSSD = true;
	}

	Input.seekg(0, std::ios::beg);
	// JGH

	FreeDiscImage(DriveNum);

	for (int Head = HeadNum; Head < Heads; Head++)
	{
		for (unsigned char Track = 0; Track < Tracks; Track++)
		{
			DiscStatus[DriveNum].Tracks[Head][Track].LogicalSectors = 10;
			DiscStatus[DriveNum].Tracks[Head][Track].NSectors = 10;
			DiscStatus[DriveNum].Tracks[Head][Track].Gap1Size = 0; // Don't bother for the mo
			DiscStatus[DriveNum].Tracks[Head][Track].Gap3Size = 0;
			DiscStatus[DriveNum].Tracks[Head][Track].Gap5Size = 0;
			DiscStatus[DriveNum].Tracks[Head][Track].TrackIsReadable = true;
			SectorType *SecPtr = DiscStatus[DriveNum].Tracks[Head][Track].Sectors = (SectorType*)calloc(10, sizeof(SectorType));

			for (unsigned char Sector = 0; Sector < 10; Sector++)
			{
				SecPtr[Sector].IDField.LogicalTrack = Track;
				SecPtr[Sector].IDField.LogicalSector = Sector;
				SecPtr[Sector].IDField.HeadNum = (unsigned char)HeadNum;
				SecPtr[Sector].IDField.SectorLength = 1; // 1 means 256 byte sectors
				SecPtr[Sector].RecordNum = Sector;
				SecPtr[Sector].Error = RESULT_REG_SUCCESS;
				SecPtr[Sector].RealSectorSize = 256;
				SecPtr[Sector].Data = (unsigned char *)calloc(1,256);
				Input.read((char*)SecPtr[Sector].Data, 256);
			}
		}
	}

	return Result;
}

/*--------------------------------------------------------------------------*/

// Load DSD image file, where track data from Head 0 and Head 1
// are interleaved.

Disc8271Result LoadSimpleDSDiscImage(const char *FileName, int DriveNum, int Tracks)
{
	std::ifstream Input(FileName, std::ios::in | std::ios::binary);

	if (!Input)
	{
		return Disc8271Result::Failed;
	}

	DiscStatus[DriveNum].NumHeads = 2; // 2 = 2 * TRACKS_PER_DRIVE DSD image

	FreeDiscImage(DriveNum);

	for (unsigned char Track = 0; Track < Tracks; Track++)
	{
		for (int Head = 0; Head < 2; Head++)
		{
			DiscStatus[DriveNum].Tracks[Head][Track].LogicalSectors = 10;
			DiscStatus[DriveNum].Tracks[Head][Track].NSectors = 10;
			DiscStatus[DriveNum].Tracks[Head][Track].Gap1Size = 0; // Don't bother for the mo
			DiscStatus[DriveNum].Tracks[Head][Track].Gap3Size = 0;
			DiscStatus[DriveNum].Tracks[Head][Track].Gap5Size = 0;
			DiscStatus[DriveNum].Tracks[Head][Track].TrackIsReadable = true;
			SectorType *SecPtr = DiscStatus[DriveNum].Tracks[Head][Track].Sectors = (SectorType *)calloc(10,sizeof(SectorType));

			for (unsigned char Sector = 0; Sector < 10; Sector++)
			{
				SecPtr[Sector].IDField.LogicalTrack = Track;
				SecPtr[Sector].IDField.LogicalSector = Sector;
				SecPtr[Sector].IDField.HeadNum = (unsigned char)Head;
				SecPtr[Sector].IDField.SectorLength = 1; // 1 means 256 byte sectors
				SecPtr[Sector].RecordNum = Sector;
				SecPtr[Sector].Error = RESULT_REG_SUCCESS;
				SecPtr[Sector].RealSectorSize = 256;
				SecPtr[Sector].Data = (unsigned char *)calloc(1,256);
				Input.read((char*)SecPtr[Sector].Data, 256);
			}
		}
	}

	return Disc8271Result::Success;
}

/*--------------------------------------------------------------------------*/

static unsigned short GetFSDSectorSize(unsigned char Index)
{
	switch (Index)
	{
		case 0:
		default:
			return 128;

		case 1:
			return 256;

		case 2:
			return 512;

		case 3:
			return 1024;

		case 4:
			return 2048;
	}
}

/*--------------------------------------------------------------------------*/

Disc8271Result LoadFSDDiscImage(const char *FileName, int DriveNum)
{
	std::ifstream Input(FileName, std::ios::in | std::ios::binary);
	Input.exceptions(std::ios::failbit | std::ios::badbit);

	if (!Input)
	{
		return Disc8271Result::Failed;
	}

	// JGH, 26-Dec-2011
	DiscStatus[DriveNum].NumHeads = 1; // 1 = TRACKS_PER_DRIVE SSD image
	                                   // 2 = 2 * TRACKS_PER_DRIVE DSD image

	const int Head = 0;

	FreeDiscImage(DriveNum);

	try
	{
		char FSDHeader[3];
		Input.read(FSDHeader, 3); // Read FSD Header

		if (FSDHeader[0] != 'F' || FSDHeader[1] != 'S' || FSDHeader[2] != 'D')
		{
			return Disc8271Result::InvalidFSD;
		}

		unsigned char Info[5];
		Input.read((char *)Info, sizeof(Info));

		#if DEBUG_8271
		const int Day = Info[0] >> 3;
		const int Month = Info[2] & 0x0F;
		const int Year = ((Info[0] & 0x07) << 8) | Info[1];

		const int CreatorID = Info[2] >> 4;
		const int Release = ((Info[4] >> 6) << 8) | Info[3];

		DebugTrace("8271: FSD File: %s, Day: %d, Month: %d, Year, %d, CreatorID: %d, Release: %d\n",
		           FileName, Day, Month, Year, CreatorID, Release);
		#endif

		std::string DiscTitle;
		char TitleChar = 1;

		while (TitleChar != '\0')
		{
			TitleChar = (char)Input.get();
			DiscTitle += TitleChar;
		}

		int LastTrack = Input.get(); // Read number of last track on disk image
		DiscStatus[DriveNum].TotalTracks = LastTrack + 1;

		if (DiscStatus[DriveNum].TotalTracks > TRACKS_PER_DRIVE)
		{
			return Disc8271Result::InvalidTracks;
		}

		for (int Track = 0; Track < DiscStatus[DriveNum].TotalTracks; Track++)
		{
			// unsigned char TrackNumber = (unsigned char)Input.get();
			Input.get(); // Read current track details
			unsigned char SectorsPerTrack = (unsigned char)Input.get(); // Read number of sectors on track
			DiscStatus[DriveNum].Tracks[Head][Track].LogicalSectors = SectorsPerTrack;

			if (SectorsPerTrack > 0) // i.e., if the track is formatted
			{
				unsigned char TrackIsReadable = (unsigned char)Input.get(); // Is track readable?
				DiscStatus[DriveNum].Tracks[Head][Track].NSectors = SectorsPerTrack; // Can be different than 10
				SectorType *SecPtr = (SectorType*)calloc(SectorsPerTrack, sizeof(SectorType));
				DiscStatus[DriveNum].Tracks[Head][Track].Sectors = SecPtr;
				DiscStatus[DriveNum].Tracks[Head][Track].TrackIsReadable = TrackIsReadable == 255;

				for (unsigned char Sector = 0; Sector < SectorsPerTrack; Sector++)
				{
					SecPtr[Sector].CylinderNum = (unsigned char)Track;

					unsigned char LogicalTrack = (unsigned char)Input.get(); // Logical track ID
					SecPtr[Sector].IDField.LogicalTrack = LogicalTrack;

					unsigned char HeadNum = (unsigned char)Input.get(); // Head number
					SecPtr[Sector].IDField.HeadNum = HeadNum;

					unsigned char LogicalSector = (unsigned char)Input.get(); // Logical sector ID
					SecPtr[Sector].IDField.LogicalSector = LogicalSector;
					SecPtr[Sector].RecordNum = Sector;

					unsigned char SectorSize = (unsigned char)Input.get(); // Reported length of sector
					SecPtr[Sector].IDField.SectorLength = SectorSize;

					if (TrackIsReadable == 255)
					{
						SectorSize = (unsigned char)Input.get(); // Real size of sector, can be misreported as copy protection
						unsigned short RealSectorSize = GetFSDSectorSize(SectorSize);

						SecPtr[Sector].RealSectorSize = RealSectorSize;

						unsigned char SectorError = (unsigned char)Input.get(); // Error code when sector was read
						SecPtr[Sector].Error = SectorError;
						SecPtr[Sector].Data = (unsigned char *)calloc(1, RealSectorSize);
						Input.read((char*)SecPtr[Sector].Data, RealSectorSize);
					}
				}
			}
		}
	}
	catch (const std::exception&)
	{
		FreeDiscImage(DriveNum);

		return Disc8271Result::Failed;
	}

	return Disc8271Result::Success;
}

/*--------------------------------------------------------------------------*/

static bool SaveTrackImage(int Drive, int Head, int Track)
{
	FILE *outfile = fopen(DiscInfo[Drive].FileName, "r+b");

	if (outfile == nullptr)
	{
#ifdef BBCA_TODO
		mainWin->Report(MessageType::Error,
		                "Could not open disc file for write:\n  %s", DiscInfo[Drive].FileName);
#endif
		return false;
	}

	long FileOffset;

	if (DiscInfo[Drive].DoubleSidedSSD)
	{
		FileOffset = (Track + Head * TRACKS_PER_DRIVE) * DFS_SECTORS_PER_TRACK * DFS_SECTOR_SIZE; // 0=2-sided SSD
	}
	else
	{
		FileOffset = (DiscStatus[Drive].NumHeads * Track + Head) * DFS_SECTORS_PER_TRACK * DFS_SECTOR_SIZE; // 1=SSD, 2=DSD
	}

	// Get the file length to check if the file needs extending
	long FileLength = 0;

	bool Success = fseek(outfile, 0L, SEEK_END) == 0;

	if (Success)
	{
		FileLength = ftell(outfile);

		if (FileLength == -1L)
		{
			Success = false;
		}
	}

	while (Success && FileOffset > FileLength)
	{
		if (fputc(0, outfile) == EOF)
		{
			Success = false;
		}

		FileLength++;
	}

	if (Success)
	{
		Success = fseek(outfile, FileOffset, SEEK_SET) == 0;

		SectorType *SecPtr = DiscStatus[Drive].Tracks[Head][Track].Sectors;

		for (int CurrentSector = 0; Success && CurrentSector < 10; CurrentSector++)
		{
			if (fwrite(SecPtr[CurrentSector].Data, 1, 256, outfile) != 256)
			{
				Success = false;
			}
		}
	}

	if (fclose(outfile) != 0)
	{
		Success = false;
	}

	if (!Success)
	{
#ifdef BBCA_TODO
		mainWin->Report(MessageType::Error,
		                "Failed writing to disc file:\n  %s", DiscInfo[Drive].FileName);
#endif
	}

	return Success;
}

/*--------------------------------------------------------------------------*/

bool IsDiscWritable(int Drive)
{
	return DiscStatus[Drive].Writeable;
}

/*--------------------------------------------------------------------------*/

// If disc is being made writable then check that the disc catalogue will
// not get corrupted if new files are added.  The files in the disc catalogue
// must be in descending sector order otherwise the DFS ROMs write over
// files at the start of the disc. The sector count in the catalogue must
// also be correct.

void DiscWriteEnable(int DriveNum, bool WriteEnable)
{
	bool DiscOK = true;

	DiscStatus[DriveNum].Writeable = WriteEnable;

	if (WriteEnable)
	{
		for (int HeadNum = 0; DiscOK && HeadNum < DiscStatus[DriveNum].NumHeads; HeadNum++)
		{
			SectorType *SecPtr = DiscStatus[DriveNum].Tracks[HeadNum][0].Sectors;

			if (SecPtr == nullptr)
			{
				return; // No disc image!
			}

			unsigned char *Data = SecPtr[1].Data;

			// Check for a Watford DFS 62 file catalogue
			int NumCatalogues = 2;

			Data = SecPtr[2].Data;

			for (int i = 0; i < 8; ++i)
			{
				if (Data[i] != (unsigned char)0xaa)
				{
					NumCatalogues = 1;
					break;
				}
			}

			for (int Catalogue = 0; DiscOK && Catalogue < NumCatalogues; ++Catalogue)
			{
				Data = SecPtr[Catalogue * 2 + 1].Data;

				// First check the number of sectors
				int NumSecs = ((Data[6] & 3) << 8) + Data[7];

				if (NumSecs != 0x320 && NumSecs != 0x190)
				{
					DiscOK = false;
				}
				else
				{
					// Now check the start sectors of each file
					int LastSec = 0x320;

					for (int File = 0; DiscOK && File < Data[5] / 8; ++File)
					{
						int StartSec = ((Data[File * 8 + 14] & 3) << 8) + Data[File * 8 + 15];

						if (LastSec < StartSec)
						{
							DiscOK = false;
						}

						LastSec = StartSec;
					}
				}
			}
		}

		if (!DiscOK)
		{
#ifdef BBCA_TODO
			mainWin->Report(MessageType::Error,
			                "WARNING - Invalid Disc Catalogue\n\n"
			                "This disc image will get corrupted if files are written to it.\n"
			                "Copy all the files to a new image to fix it.");
#endif
		}
	}
}

/*--------------------------------------------------------------------------*/

void Disc8271Reset()
{
	static bool InitialInit = true;

	FDCState.ResultReg = RESULT_REG_SUCCESS;
	FDCState.StatusReg = 0;

	UpdateNMIStatus();

	FDCState.ScanSectorNum = 0;
	FDCState.ScanCount = 0; // Read as two bytes
	FDCState.ModeReg = 0;
	FDCState.CurrentTrack[0] = 0; // 0/1 for surface number
	FDCState.CurrentTrack[1] = 0;
	FDCState.UsingSpecial = false; // FSD - Using special register
	FDCState.DriveControlOutputPort = 0;
	FDCState.DriveControlInputPort = 0;
	FDCState.BadTracks[0][0] = 0xff; // 1st subscript is surface 0/1 and second subscript is badtrack 0/1
	FDCState.BadTracks[0][1] = 0xff;
	FDCState.BadTracks[1][0] = 0xff;
	FDCState.BadTracks[1][1] = 0xff;

	// Default values set by Acorn DFS:
	FDCState.StepRate = 12;
	FDCState.HeadSettlingTime = 10;
	FDCState.IndexCountBeforeHeadUnload = 12;
	FDCState.HeadLoadTime = 8;

	if (FDCState.DriveHeadLoaded)
	{
		FDCState.DriveHeadUnloadPending = true;
		DriveHeadMotorUpdate();
	}

	ClearTrigger(Disc8271Trigger); // No Disc8271Triggered events yet

	FDCState.Command = 0;
	FDCState.CommandParamCount = 0;
	FDCState.CurrentParam = 0;
	FDCState.Select[0] = false;
	FDCState.Select[1] = false;

	if (InitialInit)
	{
		InitialInit = false;
		InitDiscStore();
	}
}

/*--------------------------------------------------------------------------*/

void Save8271UEF(FILE *SUEF)
{
#ifdef BBCA_TODO

	char blank[256];
	memset(blank,0,256);

	if (DiscStatus[0].Tracks[0][0].Sectors == nullptr)
	{
		// No disc in drive 0
		UEFWriteString("", SUEF);
	}
	else
	{
		UEFWriteString(DiscInfo[0].FileName, SUEF);
	}

	if (DiscStatus[1].Tracks[0][0].Sectors == nullptr)
	{
		// No disc in drive 1
		UEFWriteString("", SUEF);
	}
	else
	{
		UEFWriteString(DiscInfo[1].FileName, SUEF);
	}

	if (Disc8271Trigger == CycleCountTMax)
	{
		UEFWrite32(Disc8271Trigger, SUEF);
	}
	else
	{
		UEFWrite32(Disc8271Trigger - TotalCycles, SUEF);
	}

	UEFWrite8(FDCState.ResultReg, SUEF);
	UEFWrite8(FDCState.StatusReg, SUEF);
	UEFWrite8(FDCState.DataReg, SUEF);
	UEFWrite8(FDCState.ScanSectorNum, SUEF);
	UEFWrite32(FDCState.ScanCount, SUEF);
	UEFWrite8(FDCState.ModeReg, SUEF);
	UEFWrite8(FDCState.CurrentTrack[0], SUEF);
	UEFWrite8(FDCState.CurrentTrack[1], SUEF);
	UEFWrite8(FDCState.DriveControlOutputPort, SUEF);
	UEFWrite8(FDCState.DriveControlInputPort, SUEF);
	UEFWrite8(FDCState.BadTracks[0][0], SUEF);
	UEFWrite8(FDCState.BadTracks[0][1], SUEF);
	UEFWrite8(FDCState.BadTracks[1][0], SUEF);
	UEFWrite8(FDCState.BadTracks[1][1], SUEF);
	UEFWrite32(FDCState.Command,SUEF);
	UEFWrite32(FDCState.CommandParamCount, SUEF);
	UEFWrite32(FDCState.CurrentParam, SUEF);
	UEFWriteBuf(FDCState.Params, sizeof(FDCState.Params), SUEF);
	UEFWrite32(DiscStatus[0].NumHeads, SUEF);
	UEFWrite32(DiscStatus[1].NumHeads, SUEF);
	UEFWrite32(FDCState.Select[0] ? 1 : 0, SUEF);
	UEFWrite32(FDCState.Select[1] ? 1 : 0, SUEF);
	UEFWrite32(DiscStatus[0].Writeable ? 1 : 0, SUEF);
	UEFWrite32(DiscStatus[1].Writeable ? 1 : 0, SUEF);
	UEFWrite32(CommandStatus.FirstWriteInt ? 1 : 0, SUEF);
	UEFWrite32(CommandStatus.NextInterruptIsErr, SUEF);
	UEFWrite32(CommandStatus.TrackAddr, SUEF);
	UEFWrite32(CommandStatus.CurrentSector, SUEF);
	UEFWrite32(CommandStatus.SectorLength, SUEF);
	UEFWrite32(CommandStatus.SectorsToGo, SUEF);
	UEFWrite32(CommandStatus.ByteWithinSector, SUEF);

	UEFWrite8(FDCState.StepRate, SUEF);
	UEFWrite8(FDCState.HeadSettlingTime, SUEF);
	UEFWrite8(FDCState.IndexCountBeforeHeadUnload, SUEF);
	UEFWrite8(FDCState.HeadLoadTime, SUEF);
	UEFWrite8(FDCState.DriveHeadLoaded, SUEF);
	UEFWrite8(FDCState.DriveHeadUnloadPending, SUEF);

	// Load FSD-specific state
	UEFWrite8(FDCState.DriveHeadPosition[0], SUEF);
	UEFWrite8(FDCState.DriveHeadPosition[1], SUEF);
	UEFWrite8(FDCState.PositionInTrack[0], SUEF);
	UEFWrite8(FDCState.PositionInTrack[1], SUEF);
	UEFWrite8(FDCState.SectorOverRead, SUEF);
	UEFWrite8(FDCState.UsingSpecial, SUEF);
	UEFWrite8(FDCState.DRDSC, SUEF);
	UEFWrite8(FDCState.FSDLogicalTrack[0], SUEF);
	UEFWrite8(FDCState.FSDLogicalTrack[1], SUEF);
	UEFWrite8(FDCState.FSDPhysicalTrack[0], SUEF);
	UEFWrite8(FDCState.FSDPhysicalTrack[1], SUEF);
#endif
}

void Load8271UEF(FILE *SUEF, int Version)
{
#ifdef BBCA_TODO
	bool Loaded = false;
	bool LoadSuccess = true;

	// Clear out current images, don't want them corrupted if
	// saved state was in middle of writing to disc.
	FreeDiscImage(0);
	FreeDiscImage(1);

	DiscInfo[0].Loaded = false;
	DiscInfo[1].Loaded = false;

	char FileName[256];
	memset(FileName, 0, sizeof(FileName));

	if (Version >= 14)
	{
		UEFReadString(FileName, sizeof(FileName), SUEF);
	}
	else
	{
		UEFReadBuf(FileName, sizeof(FileName), SUEF);
	}

	if (FileName[0] != '\0')
	{
		// Load drive 0
		Loaded = true;

		const char *ext = strrchr(FileName, '.');

		if (ext != nullptr && StrCaseCmp(ext + 1, "dsd") == 0)
		{
			LoadSuccess = mainWin->Load8271DiscImage(FileName, 0, 80, DiscType::DSD);
		}
		else if (Version >= 14 && ext != nullptr && StrCaseCmp(ext + 1, "fsd") == 0)
		{
			LoadSuccess = mainWin->Load8271DiscImage(FileName, 0, 80, DiscType::FSD);
		}
		else
		{
			LoadSuccess = mainWin->Load8271DiscImage(FileName, 0, 80, DiscType::SSD);
		}
	}

	memset(FileName, 0, sizeof(FileName));

	if (Version >= 14)
	{
		UEFReadString(FileName, sizeof(FileName), SUEF);
	}
	else
	{
		UEFReadBuf(FileName, sizeof(FileName), SUEF);
	}

	if (FileName[0] != '\0')
	{
		// Load drive 1
		Loaded = true;

		const char *ext = strrchr(FileName, '.');

		if (ext != nullptr && StrCaseCmp(ext + 1, "dsd") == 0)
		{
			LoadSuccess = mainWin->Load8271DiscImage(FileName, 1, 80, DiscType::DSD);
		}
		else if (Version >= 14 && ext != nullptr && StrCaseCmp(ext + 1, "fsd") == 0)
		{
			LoadSuccess = mainWin->Load8271DiscImage(FileName, 1, 80, DiscType::FSD);
		}
		else
		{
			LoadSuccess = mainWin->Load8271DiscImage(FileName, 1, 80, DiscType::SSD);
		}
	}

	if (Loaded && LoadSuccess)
	{
		Disc8271Trigger = UEFRead32(SUEF);
		if (Disc8271Trigger != CycleCountTMax)
			Disc8271Trigger += TotalCycles;

		FDCState.ResultReg = UEFRead8(SUEF);
		FDCState.StatusReg = UEFRead8(SUEF);
		FDCState.DataReg = UEFRead8(SUEF);
		FDCState.ScanSectorNum = UEFRead8(SUEF);
		FDCState.ScanCount = UEFRead32(SUEF);
		FDCState.ModeReg = UEFRead8(SUEF);
		FDCState.CurrentTrack[0] = UEFRead8(SUEF);
		FDCState.CurrentTrack[1] = UEFRead8(SUEF);
		FDCState.DriveControlOutputPort = UEFRead8(SUEF);
		FDCState.DriveControlInputPort = UEFRead8(SUEF);
		FDCState.BadTracks[0][0] = UEFRead8(SUEF);
		FDCState.BadTracks[0][1] = UEFRead8(SUEF);
		FDCState.BadTracks[1][0] = UEFRead8(SUEF);
		FDCState.BadTracks[1][1] = UEFRead8(SUEF);
		FDCState.Command = (unsigned char)UEFRead32(SUEF);
		FDCState.CommandParamCount = UEFRead32(SUEF);
		FDCState.CurrentParam = UEFRead32(SUEF);
		UEFReadBuf(FDCState.Params, sizeof(FDCState.Params), SUEF);
		DiscStatus[0].NumHeads = UEFRead32(SUEF);
		DiscStatus[1].NumHeads = UEFRead32(SUEF);
		FDCState.Select[0] = UEFRead32(SUEF) != 0;
		FDCState.Select[1] = UEFRead32(SUEF) != 0;
		DiscStatus[0].Writeable = UEFRead32(SUEF) != 0;
		DiscStatus[1].Writeable = UEFRead32(SUEF) != 0;
		CommandStatus.FirstWriteInt = UEFRead32(SUEF) != 0;
		CommandStatus.NextInterruptIsErr = (unsigned char)UEFRead32(SUEF);
		CommandStatus.TrackAddr = (unsigned char)UEFRead32(SUEF);
		CommandStatus.CurrentSector = (unsigned char)UEFRead32(SUEF);
		CommandStatus.SectorLength = UEFRead32(SUEF);
		CommandStatus.SectorsToGo = UEFRead32(SUEF);
		CommandStatus.ByteWithinSector = UEFRead32(SUEF);

		if (Version >= 14)
		{
			// Load 8271 state
			FDCState.StepRate = UEFRead8(SUEF);
			FDCState.HeadSettlingTime = UEFRead8(SUEF);
			FDCState.IndexCountBeforeHeadUnload = UEFRead8(SUEF);
			FDCState.HeadLoadTime = UEFRead8(SUEF);
			FDCState.DriveHeadLoaded = UEFRead8(SUEF) != 0;
			FDCState.DriveHeadUnloadPending = UEFRead8(SUEF) != 0;

			// Load FSD-specific state
			FDCState.DriveHeadPosition[0] = UEFRead8(SUEF);
			FDCState.DriveHeadPosition[1] = UEFRead8(SUEF);
			FDCState.PositionInTrack[0] = UEFRead8(SUEF);
			FDCState.PositionInTrack[1] = UEFRead8(SUEF);
			FDCState.SectorOverRead = UEFRead8(SUEF) != 0;
			FDCState.UsingSpecial = UEFRead8(SUEF) != 0;
			FDCState.DRDSC = UEFRead8(SUEF);
			FDCState.FSDLogicalTrack[0] = UEFRead8(SUEF);
			FDCState.FSDLogicalTrack[1] = UEFRead8(SUEF);
			FDCState.FSDPhysicalTrack[0] = UEFRead8(SUEF);
			FDCState.FSDPhysicalTrack[1] = UEFRead8(SUEF);
		}

		CommandStatus.CurrentTrackPtr = GetTrackPtr(CommandStatus.TrackAddr);

		if (CommandStatus.CurrentTrackPtr != nullptr)
		{
			CommandStatus.CurrentSectorPtr = GetSectorPtr(CommandStatus.CurrentTrackPtr,
			                                              CommandStatus.CurrentSector,
			                                              false);
		}
		else
		{
			CommandStatus.CurrentSectorPtr = nullptr;
		}
	}
#endif
}
