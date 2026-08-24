#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FRecentMemoryAccessViewer : public FViewerBase
{
public:
	FRecentMemoryAccessViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

private:
	bool bTrackStack = false;
	FPCEEmu* pPCEEmu = nullptr;
};
