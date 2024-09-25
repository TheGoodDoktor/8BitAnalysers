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
	// constants
	static const uint32_t kCodeCol = 0xff008080;
	static const uint32_t kCodeColActive = 0xff00ffff;
	static const uint32_t kDataReadActiveCol = 0xff00ff00;
	static const uint32_t kDataReadCol = 0xff008000;
	static const uint32_t kDataWriteActiveCol = 0xff0000ff;
	static const uint32_t kDataWriteCol = 0xff000080;
	static const uint32_t kDefaultDataCol = 0xffff0000;
	static const uint32_t kBitmapDataCol = 0xffffffff;
	static const uint32_t kCharMapDataCol = 0xff00ff00;
	static const uint32_t kTextDataCol = 0xffff00ff;
	static const uint32_t kScreenPixelsDataCol = 0xffff80ff;
	static const uint32_t kColAttribDataCol = 0xff0080ff;
	static const uint32_t kUnknownDataCol = 0xff808080;

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
	
	void	DrawLegend(void);

private:
	FOverviewStats	Stats;
	int16_t		OverviewBankId = -1;

	bool		bShowActivity = true;
	FGraphicsView*	MemoryViewImage = nullptr;
	bool		bShowROM = false;
	bool		bShowCurrentLocation = true;
};