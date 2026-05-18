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
	// constants
	static const uint32_t kUnknownWriteActiveCol = 0xff0000ff;
	static const uint32_t kUnknownWriteCol        = 0xff000080;

	static const uint32_t kSpriteWriteActiveCol   = 0xffffff00;
	static const uint32_t kSpriteWriteCol         = 0xff808000;

	static const uint32_t kBGTileWriteActiveCol   = 0xff00c800;  // bright green
	static const uint32_t kBGTileWriteCol         = 0xff006400;  // dark green

	static const uint32_t kBATWriteActiveCol      = 0xff0080ff;  // bright orange
	static const uint32_t kBATWriteCol            = 0xff004080;  // dark orange

	static const uint32_t kSATWriteActiveCol      = 0xffc000c0;  // bright magenta
	static const uint32_t kSATWriteCol            = 0xff600060;  // dark magenta

	static const uint32_t kUnwrittenCol           = 0xff808080;

	FVRAMViewer(FEmuBase* pEmu);

	bool	Init(void) override;
	void	Shutdown() override {}
	void	DrawUI(void) override;
	void	Tick();

	void	Reset();
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
	int GetBGTileIndexForAddress(uint16_t addr) const { return BGTileLookup[addr]; }
	int GetBATIndexForAddress(uint16_t addr) const    { return BATLookup[addr]; }
	int GetSATWordForAddress(uint16_t addr) const     { return SATWordLookup[addr]; }
	int GetSpriteHighlight() const { return SpriteHighlight; }
	const FVRAMAccess& GetVRAMAccess(uint16_t vramAddr) const { return Access[vramAddr]; }

private:

	FGraphicsView*	MemoryViewImage  = nullptr;
	FGraphicsView*	BGTileViewImage  = nullptr;
	FGraphicsView*	SpriteViewImage  = nullptr;

	int             BGTileOffset     = 0;   // starting tile index (0–2047)
	int             BGTilePalette    = 0;   // BG palette 0–15
	int             BGTileScale      = 1;

	int             SpriteBlockOffset = 0;  // starting 64-word sprite block index
	int             SpritePalette     = 0;  // sprite palette 0–15 (maps to colourTable[0x100+])
	int             SpriteScale       = 1;

	FVRAMAccess Access[HUC6270_VRAM_SIZE];
	FSpriteInfo SpriteInfo[HUC6270_SPRITES];
	int16_t     SpriteIndexLookup[HUC6270_VRAM_SIZE];
	int16_t     BGTileLookup[HUC6270_VRAM_SIZE];   // tile index (0-2047) or -1
	int16_t     BATLookup[HUC6270_VRAM_SIZE];       // BAT entry index or -1
	int16_t     SATWordLookup[HUC6270_VRAM_SIZE];   // sprite*4+field (0-255) or -1

	FPCEEmu* pPCEEmu = nullptr;

	int SpriteHighlight = -1;
};