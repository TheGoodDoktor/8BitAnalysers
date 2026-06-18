#pragma once

#include "huc6270_defines.h"
#include "CodeAnalyser/CodeAnalyserTypes.h"

class FPCEEmu;

struct FVRAMAccess
{
	FAddressRef LastWriter       = FAddressRef::Invalid();
	int         FrameLastWritten = -1;
	FAddressRef LastReader       = FAddressRef::Invalid();
	int         FrameLastRead    = -1;
};

struct FSpriteInfo
{
	uint16_t XPos        = 0;
	uint16_t YPos        = 0;
	uint16_t Address     = 0;
	uint16_t SizeInBytes = 0;
	int      Width       = 0;
	int      Height      = 0;
	int      Palette     = -1;
};

class FVRAMAnalysisState
{
public:
	FVRAMAnalysisState(FPCEEmu* pEmu);

	void Tick();
	void Reset();
	void ClearUsage();
	void RegisterRead(uint16_t vramAddress, FAddressRef reader);
	void RegisterWrite(uint16_t vramAddress, FAddressRef writer);

	int GetSpriteIndexForAddress(uint16_t addr) const { return SpriteIndexLookup[addr]; }
	const FSpriteInfo* GetSpriteInfo() const { return SpriteInfo; }
	int                GetSpriteCount() const { return HUC6270_SPRITES; }
	const FVRAMAccess& GetVRAMAccess(uint16_t addr) const { return Access[addr]; }

	FAddressRef GetLastVRAMWriter() const { return LastVRAMWriter; }
	int         GetLastVRAMWriteFrame() const { return LastVRAMWriteFrame; }
	FAddressRef GetLastVRAMReader() const { return LastVRAMReader; }
	int         GetLastVRAMReadFrame() const { return LastVRAMReadFrame; }

private:
	FAddressRef LastVRAMWriter     = FAddressRef::Invalid();
	int         LastVRAMWriteFrame = -1;
	FAddressRef LastVRAMReader     = FAddressRef::Invalid();
	int         LastVRAMReadFrame  = -1;

	FPCEEmu*    pPCEEmu = nullptr;
	FVRAMAccess Access[HUC6270_VRAM_SIZE];
	FSpriteInfo SpriteInfo[HUC6270_SPRITES];
	int16_t     SpriteIndexLookup[HUC6270_VRAM_SIZE];
};
