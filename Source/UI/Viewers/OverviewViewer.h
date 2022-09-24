#pragma once

#include "ViewerBase.h"

class FSpectrumEmu;

class FOverviewViewer : public FViewerBase
{
public:
			FOverviewViewer(FSpectrumEmu* pEmu) : FViewerBase(pEmu) { Name = "Overview"; }

	bool	Init(void) override { return true; }
	void	DrawUI(void) override;
private:
};