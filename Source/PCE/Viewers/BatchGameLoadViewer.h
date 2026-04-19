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
	float GetElapsedGameRunTime() const { return ElapsedGameRunTime; }
	int GetTestingMethodology() const;

private:
	double GetNextButtonPressTime() const;

	void StartAutomation();

private:
	bool bAutomationActive = false;
	bool bLoadGame = false;
	bool bPressRandomButtons = false;
	bool bLoadExistingProject = false;
	bool bExportAsm = false;
	bool bSkipWhenMapped = false;
	double NextButtonPressTime = DBL_MAX;
	float InputDelay = 0.5f;
	int GameIndex = 0;
	int GameRunTime = 10;
	bool bUseFramesForRunTime = false;
	int GameFrameCount = 0;
	float ElapsedGameRunTime = 0.f;
	int TimeUntilButtonPresses = 0;
	double NextGameTime = DBL_MAX;
	double AutomationStartTime = 0.0;

	FPCEEmu* pPCEEmu = nullptr;
};
