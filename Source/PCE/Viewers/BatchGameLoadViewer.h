#pragma once

#include "imgui.h"
#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FBatchGameLoadViewer : public FViewerBase
{
public:
	FBatchGameLoadViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

private:

	bool bAutomationActive = false;
	int GameRunTime = 10;
	double NextGameTime = DBL_MAX;

	FPCEEmu* pPCEEmu = nullptr;
};
