#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"

class FPCEEmu;

class FBatchGameLoadViewer : public FViewerBase
{
public:
	FBatchGameLoadViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

	bool IsAutomationActive() const { return bAutomationActive; }

private:
	double GetNextButtonPressTime() const;

	void StartAutomation();

private:
	bool bAutomationActive = false;
	bool bLoadGame = false;
	bool bPressRandomButtons = false;
	bool bLoadExistingProject = false;
	double NextButtonPressTime = DBL_MAX;
	float InputDelay = 0.5f;
	int GameIndex = 0;
	int GameRunTime = 10;
	double NextGameTime = DBL_MAX;

	FPCEEmu* pPCEEmu = nullptr;
};
