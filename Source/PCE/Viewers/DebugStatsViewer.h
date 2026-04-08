#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"
#include <map>

class FPCEEmu;

class FDebugStatsViewer : public FViewerBase
{
public:
	FDebugStatsViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

protected:
	void DrawBankSets();
	void DrawDebugStatsTable();
	void DrawGeneralStats();
	void DrawBankList();

protected:
	bool bDumpBanks = false;
	bool bOnlyShowProblemLabels = false;
	
	std::map<std::string, float> TimeUntilMapped;

	FPCEEmu* pPCEEmu = nullptr;
};
