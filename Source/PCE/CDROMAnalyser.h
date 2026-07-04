#pragma once

class FPCEEmu;

enum class EBiosCDReadDest
{
	PhysicalMemory,
	VRAM,
	MPRBank,
};

struct FBiosCDReadArgs
{
	EBiosCDReadDest Destination;
	uint32_t NumToRead = 0;
	uint16_t PhysicalAddr = 0;
	uint32_t CDByteOffset = 0;
};

class FCDROMAnalyser
{
public:
	FCDROMAnalyser(FPCEEmu* pEmu);

private:
	FPCEEmu*    pPCEEmu = nullptr;
};

bool GetBiosCDReadArgs(FPCEEmu* pEmu, FBiosCDReadArgs& args);