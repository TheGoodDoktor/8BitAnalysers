#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FDebugStatsViewer : public FViewerBase
{
public:
	FDebugStatsViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

	FPCEEmu* pPCEEmu = nullptr;
};
