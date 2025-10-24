#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"
#include "huc6270_defines.h"

class FPCEEmu;
class FCodeAnalysisState;
class FGraphicsView;

struct FVRAMAccess
{
	int FrameLastWritten = -1;
};

class FVRAMViewer : public FViewerBase
{
public:
	// constants
	static const uint32_t kDataWriteActiveCol = 0xff0000ff;
	static const uint32_t kDataWriteCol = 0xff000080;
	static const uint32_t kDefaultDataCol = 0xffff0000;

	FVRAMViewer(FEmuBase* pEmu);

	bool	Init(void) override;
	void	Shutdown() override {}
	void	DrawUI(void) override;

	void	DrawBankOverview();
	void	DrawPhysicalMemoryOverview();

	void	DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix);
	
	void	DrawLegend(void);

	void	RegisterAccess(uint16_t address);
private:

	FGraphicsView*	MemoryViewImage = nullptr;

	FVRAMAccess Access[HUC6270_VRAM_SIZE];

	FPCEEmu* pPCEEmu = nullptr;
};