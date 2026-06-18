#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"
#include "../VRAMAnalyser.h"

class FPCEEmu;
class FCodeAnalysisState;
class FGraphicsView;

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

	void	DrawBankOverview();
	void	DrawPhysicalMemoryOverview();
	void	DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix);
	void	DrawLegend(void);
	void	DrawBGTileView(void);
	void	DrawSpriteView(void);

	void	ClearUsage();

private:

	FGraphicsView*	MemoryViewImage  = nullptr;
	FGraphicsView*	BGTileViewImage  = nullptr;
	FGraphicsView*	SpriteViewImage  = nullptr;

	int             BGTileOffset     = 0;   // starting tile index (0–2047)
	int             BGTilePalette    = 0;   // BG palette 0–15
	int             BGTileScale      = 1;
	int             BGTileViewRows   = 32;  // visible tile rows (8–64)
	bool            bPreviewPalette  = true;
	bool            bBGTileGreyscale = false;

	int             SpriteBlockOffset = 0;  // starting 64-word sprite block index
	int             SpritePalette     = 16; // sprite palette store index 16–31
	int             SpriteScale       = 1;
	int             SpriteViewRows    = 16; // visible sprite block rows (4–32)
	bool            bShowWriterSnippet = false;
	bool            bSpriteGreyscale  = false;

	FPCEEmu* pPCEEmu = nullptr;
};
