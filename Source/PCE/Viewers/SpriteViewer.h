
#pragma once

#include "imgui.h"

#include "CodeAnalyser/UI/ViewerBase.h"
#include "CodeAnalyser/CodeAnalyserTypes.h"

#include <vector>

class FPCEEmu;

struct FHistorySpriteEntry
{
	uint8_t*    PixelBuffer      = nullptr;  // RGBA snapshot (Width * Height * 4 bytes)
	uint8_t*    VRAMSnapshot     = nullptr;  // raw VRAM bytes used for Find in Memory search
	int         VRAMSnapshotSize = 0;
	ImTextureID Texture          = 0;
	int         Width            = 0;
	int         Height           = 0;
	int         Palette          = 0;
	uint16_t    VRAMAddress      = 0;
	uint32_t    DataHash         = 0;        // FNV-1a of VRAM bytes — dedup key
	FAddressRef FoundDataAddr = FAddressRef::Invalid();
	bool        bFormatted       = false;
};

class FSpriteViewer : public FViewerBase
{
public:
	FSpriteViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override;
	virtual void ResetForGame() override;
	virtual void DrawUI() override;
	void Tick();

	ImTextureID GetSpriteTexture(int spriteIndex) const { return SpriteTextures[spriteIndex]; }
	int CountSpritesFoundInMemory(int& outHistorySize);

	FPCEEmu* pPCEEmu = nullptr;
public:
	int HighlightSprite = -1;
	int SelectedSprite = -1;

private:
	void DrawSpriteDetails(int spriteIndex);
	void ResetScreenTexture();
	void UpdateSpriteBuffers();
	void UpdateSpriteHistory();
	void ClearHistory();
	void DrawHistoryTab();
	void DrawHistoryDetails(int index);
	void DrawResultsTab();
	void FormatEntry(FHistorySpriteEntry& e);

private:
	static const int kNumSprites = 64;
	static const int kMaxHistoryEntries = 512;

	ImTextureID	SpriteTextures[kNumSprites] = { 0 };
	uint8_t* SpriteBuffers[kNumSprites];

	ImTextureID	ScreenTexture = 0;
	int TextureWidth = 0;
	int TextureHeight = 0;

	bool bShowMagnifier = true;
	int BackgroundColour = 0;

	FAddressRef FoundSpriteDataAddr;
	bool        bFoundSpriteData = false;
	int         LastSearchedSprite = -1;

	// History tab
	std::vector<FHistorySpriteEntry> SpriteHistory;
	uint32_t SlotHash[kNumSprites] = {};
	int HistorySelectedSprite = -1;
	int FindCursor = -1;
	int FindFound  = 0;
	int FindTotal  = 0;
};
