#pragma once

#include <vector>
#include "CodeAnalyser/CodeAnalyserTypes.h"
class FPCEEmu;

enum class EBiosCDReadDest
{
	PhysicalMemory,
	VRAM,
	MPRBank,
	Unknown
};

struct FBiosCDReadArgs
{
	EBiosCDReadDest Destination = EBiosCDReadDest::Unknown;
	uint32_t TransferSizeBytes = 0;
	uint16_t PhysicalAddr = 0;
	uint32_t CDByteOffset = 0;
};

struct FCDROMTransfer
{
	uint32_t CDByteOffset = 0; // is this better in cd sectors?
	FAddressRef DestAddr = FAddressRef::Invalid(); // start address of where the data was written to in memory.
	uint16_t SizeInBytes = 0; // size of data that was transferred.
	int TrackIndex = -1; // 0-based index into the disc TOC
};

typedef std::vector<FCDROMTransfer> TCDTransferList;

class FCDROMAnalyser
{
public:
	FCDROMAnalyser(FPCEEmu* pEmu);

	void RegisterCDRead(const FBiosCDReadArgs& args);

private:
	std::vector<TCDTransferList> TrackTransfers; // one list per data track, indexed by track number

	FPCEEmu*    pPCEEmu = nullptr;
};

bool GetBiosCDReadArgs(FPCEEmu* pEmu, FBiosCDReadArgs& args);