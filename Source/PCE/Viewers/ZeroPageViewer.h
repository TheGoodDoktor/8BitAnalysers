#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FZeroPageViewer : public FViewerBase
{
public:
	FZeroPageViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

private:
	FPCEEmu* pPCEEmu          = nullptr;
	bool     bHideUnused      = true;
	int      unusedFrameLimit = 300;
	int      resetFrameNo          = -1;
	int      resetExecutionCount   = -1;
};
