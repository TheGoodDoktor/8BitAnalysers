#pragma once

#include "ViewerBase.h"

class FBreakpointViewer : public FViewerBase
{
public:
			FBreakpointViewer(FSpectrumEmu* pEmu) :FViewerBase(pEmu) { Name = "Breakpoints"; }
	bool	Init(void) override { return true; }
	void	DrawUI() override;
private:
};