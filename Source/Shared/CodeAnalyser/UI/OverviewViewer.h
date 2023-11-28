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

	bool	Init(void) override { return true; }
	void	Shutdown() override {}
	void	DrawUI(void) override;

	void	DrawStats();
	void	CalculateStats();
private:
	FOverviewStats	Stats;
};