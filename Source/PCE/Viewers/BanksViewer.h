#pragma once

#include "CodeAnalyser/UI/ViewerBase.h"
#include "../BankSet.h"

#include <vector>

class FPCEEmu;
struct FCodeAnalysisBank;

class FBanksViewer : public FViewerBase
{
public:
	FBanksViewer(FEmuBase* pEmu);

	virtual bool Init() override;
	virtual void Shutdown() override {}
	virtual void DrawUI() override;

	void DrawBankTable(const std::vector<FCodeAnalysisBank*>& Banks, const std::vector<FBankSet*>& BankSets);
	void DrawBankDetail(const FBankSet* pBankSet, const FCodeAnalysisBank* pBank);

	FPCEEmu*	pPCEEmu = nullptr;
	int			SelectedBankIdx = -1;
};
