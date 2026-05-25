#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"
#include "huc6270_defines.h"
#include "CodeAnalyser/CodeAnalyserTypes.h"

class FPCEEmu;
class FCodeAnalysisState;
class FGraphicsView;

struct FVRAMAccess
{
	FAddressRef LastWriter;
	int FrameLastWritten = -1;
	FAddressRef LastReader;
	int FrameLastRead = -1;
};

struct FSpriteInfo
{
	uint16_t XPos = 0;
	uint16_t YPos = 0;
	uint16_t Address = 0;
	uint16_t SizeInBytes = 0;
	int Width = 0;
	int Height = 0;
	int Palette = -1;
};

class FVRAMViewer : public FViewerBase
{
public:
	// colours match OverviewViewer scheme
	static const uint32_t kUnwrittenCol          = 0xff808080;  // grey  - never accessed
	static const uint32_t kUnknownWriteCol       = 0xff000080;  // dark red  - previous write
	static const uint32_t kUnknownWriteActiveCol = 0xff0000ff;  // bright red - active write
	static const uint32_t kDataReadCol           = 0xff008000;  // dark green - previous read
	static const uint32_t kDataReadActiveCol     = 0xff00ff00;  // bright green - active read

	FVRAMViewer(FEmuBase* pEmu);

	bool	Init(void) override;
	void	Shutdown() override {}
	void	DrawUI(void) override;
	void	Tick();

	void	Reset();
	void	ClearUsage();
	void	DrawBankOverview();
	void	DrawPhysicalMemoryOverview();
	void	DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix);
	void	DrawLegend(void);
	void	DrawBGTileView(void);
	void	DrawSpriteView(void);

	void	RegisterRead(uint16_t vramAddress, FAddressRef reader);
	void	RegisterWrite(uint16_t vramAddress, FAddressRef writer);

	const FSpriteInfo* GetSpriteInfo() const { return SpriteInfo; }
	int GetSpriteIndexForAddress(uint16_t addr) const;
	int GetSpriteHighlight() const { return SpriteHighlight; }
	const FVRAMAccess& GetVRAMAccess(uint16_t vramAddr) const { return Access[vramAddr]; }

private:

	FGraphicsView*	MemoryViewImage  = nullptr;
	FGraphicsView*	BGTileViewImage  = nullptr;
	FGraphicsView*	SpriteViewImage  = nullptr;

	int             BGTileOffset     = 0;   // starting tile index (0–2047)
	int             BGTilePalette    = 0;   // BG palette 0–15
	int             BGTileScale      = 1;
	int             BGTileViewRows   = 32;  // visible tile rows (8–64)

	int             SpriteBlockOffset = 0;  // starting 64-word sprite block index
	int             SpritePalette     = 0;  // sprite palette 0–15 (maps to colourTable[0x100+])
	int             SpriteScale       = 1;
	int             SpriteViewRows    = 16; // visible sprite block rows (4–32)

	FVRAMAccess Access[HUC6270_VRAM_SIZE];
	FSpriteInfo SpriteInfo[HUC6270_SPRITES];
	int16_t     SpriteIndexLookup[HUC6270_VRAM_SIZE];

	FAddressRef LastVRAMWriter;
	int         LastVRAMWriteFrame = -1;
	FAddressRef LastVRAMReader;
	int         LastVRAMReadFrame  = -1;

	FPCEEmu* pPCEEmu = nullptr;

	int SpriteHighlight = -1;
};