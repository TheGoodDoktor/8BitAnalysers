#include "VRAMAnalyser.h"
#include "PCEEmu.h"

#include <geargrafx_core.h>
#include <string.h>

FVRAMAnalysisState::FVRAMAnalysisState(FPCEEmu* pEmu)
	: pPCEEmu(pEmu)
{
}

void FVRAMAnalysisState::Reset()
{
	ClearUsage();
	memset(SpriteIndexLookup, -1, sizeof(SpriteIndexLookup));
}

void FVRAMAnalysisState::ClearUsage()
{
	for (FVRAMAccess& access : Access)
	{
		access.FrameLastWritten = -1;
		access.FrameLastRead    = -1;
		access.LastWriter       = FAddressRef::Invalid();
		access.LastReader       = FAddressRef::Invalid();
	}
	LastVRAMWriter     = FAddressRef::Invalid();
	LastVRAMWriteFrame = -1;
	LastVRAMReader     = FAddressRef::Invalid();
	LastVRAMReadFrame  = -1;
}

void FVRAMAnalysisState::RegisterRead(uint16_t vramAddress, FAddressRef reader)
{
	if (vramAddress < HUC6270_VRAM_SIZE)
	{
		const int frameNo       = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
		FVRAMAccess& access     = Access[vramAddress];
		access.FrameLastRead    = frameNo;
		access.LastReader       = reader;
		LastVRAMReader          = reader;
		LastVRAMReadFrame       = frameNo;
	}
}

void FVRAMAnalysisState::RegisterWrite(uint16_t vramAddress, FAddressRef writer)
{
	if (vramAddress < HUC6270_VRAM_SIZE)
	{
		const int frameNo       = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
		FVRAMAccess& access     = Access[vramAddress];
		access.FrameLastWritten = frameNo;
		access.LastWriter       = writer;
		LastVRAMWriter          = writer;
		LastVRAMWriteFrame      = frameNo;
	}
}

void FVRAMAnalysisState::Tick()
{
	HuC6270* huc6270 = pPCEEmu->GetCore()->GetHuC6270_1();
	u16* sat  = huc6270->GetSAT();

	for (int i = 0; i < HUC6270_SPRITES; i++)
	{
		const int sprite_offset = i << 2;
		u16* spriteSat = sat + sprite_offset;

		const u16 flags = spriteSat[3] & 0xB98F;
		const int cgx   = (flags >> 8) & 0x01;
		const int cgy   = (flags >> 12) & 0x03;
		SpriteInfo[i].Width       = k_huc6270_sprite_width[cgx];
		SpriteInfo[i].Height      = k_huc6270_sprite_height[cgy];
		SpriteInfo[i].SizeInBytes = (SpriteInfo[i].Width * SpriteInfo[i].Height) >> 1;
		SpriteInfo[i].Palette     = flags & 0x0F;

		SpriteInfo[i].XPos = spriteSat[0];
		SpriteInfo[i].YPos = spriteSat[1];

		u16 pattern = (spriteSat[2] >> 1) & 0x3FF;
		pattern &= k_huc6270_sprite_mask_width[cgx];
		pattern &= k_huc6270_sprite_mask_height[cgy];
		SpriteInfo[i].Address = pattern << 6;
	}

	memset(SpriteIndexLookup, -1, sizeof(SpriteIndexLookup));
	for (int i = 0; i < HUC6270_SPRITES; i++)
	{
		const uint16_t start = SpriteInfo[i].Address;
		const uint16_t end   = start + SpriteInfo[i].SizeInBytes / 2;
		for (uint16_t addr = start + 1; addr < end && addr < HUC6270_VRAM_SIZE; addr++)
			SpriteIndexLookup[addr] = (int16_t)i;
	}
}
