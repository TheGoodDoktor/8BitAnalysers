#pragma once

#include "Misc/EmuBase.h"

class FSpectrumEmu;

struct FOverviewStats
{
	int UnCommentedCodeCount = 0;
	int CommentedCodeCount = 0;
	int ReadOnlyDataCount = 0;
	int WriteOnlyDataCount = 0;
	int ReadWriteDataCount = 0;
	int UnknownCount = 0;
	int TotalItems = 0;

	int NoCodeItems() const {	return CommentedCodeCount + UnCommentedCodeCount;	}

	float PercentReadOnlyData = 0.0f;
	float PercentReadWriteData = 0.0f;
	float PercentWriteOnlyData = 0.0f;
	float PercentCommentedCode = 0.0f;
	float PercentUncommentedCode = 0.0f;
	float PercentUnknown = 0.0f;
};

class FOverviewViewer : public FViewerBase
{
public:
			FOverviewViewer(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Overview"; }

	bool	Init(void) override;
	void	Shutdown() override {}
	void	DrawUI(void) override;

	void	DrawStats();
	void	CalculateStats();

	void	DrawBankOverview();
	void	DrawPhysicalMemoryOverview();

	void	DrawAccessMap(FCodeAnalysisState& state, uint32_t* pPix);
	void	DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix);

private:
	FOverviewStats	Stats;
	int16_t		OverviewBankId = -1;

	FGraphicsView*	MemoryViewImage = nullptr;
};