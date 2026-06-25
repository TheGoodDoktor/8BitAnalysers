#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;
class FCodeAnalysisState;
class FGraphicsView;

class FCDROMViewer : public FViewerBase
{
public:
	// colours match OverviewViewer scheme
	static const uint32_t kUnwrittenCol          = 0xff808080;  // grey  - never accessed
	static const uint32_t kDataReadCol           = 0xff008000;  // dark green - previous read
	static const uint32_t kDataReadActiveCol     = 0xff00ff00;  // bright green - active read

	FCDROMViewer(FEmuBase* pEmu);

	bool	Init(void) override;
	void	Shutdown() override {}
	void	DrawUI(void) override;

	void	DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix);
	void	DrawLegend(void);

	void	ClearUsage();

private:

	FGraphicsView*	MemoryViewImage  = nullptr;

	FPCEEmu* pPCEEmu = nullptr;
};
