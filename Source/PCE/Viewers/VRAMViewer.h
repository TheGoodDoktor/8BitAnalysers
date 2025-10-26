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
};

struct FSpriteInfo
{
	uint16_t Address = 0;
	uint16_t SizeInBytes = 0;
	int Width = 0;
	int Height = 0;
};

class FVRAMViewer : public FViewerBase
{
public:
	// constants
	static const uint32_t kUnknownWriteActiveCol = 0xff0000ff;
	static const uint32_t kUnknownWriteCol = 0xff000080;
	
	static const uint32_t kSpriteWriteActiveCol = 0xffffff00;
	static const uint32_t kSpriteWriteCol = 0xff808000;

	static const uint32_t kUnwrittenCol = 0xff808080;

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

	void	RegisterAccess(uint16_t vramAddress, FAddressRef writer);

	const FSpriteInfo* GetSpriteInfo() const { return SpriteInfo; }
	int GetSpriteIndexForAddress(uint16_t addr) const;
	int GetSpriteHighlight() const { return SpriteHighlight; }

private:

	FGraphicsView*	MemoryViewImage = nullptr;

	FVRAMAccess Access[HUC6270_VRAM_SIZE];
	FSpriteInfo SpriteInfo[HUC6270_SPRITES];

	FPCEEmu* pPCEEmu = nullptr;

	int SpriteHighlight = -1;
};