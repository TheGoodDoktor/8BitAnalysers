#pragma once

#include <vector>
#include <string>
#include "CodeAnalyserTypes.h"
#include "UI/ViewerBase.h"


class FCodeAnalysisState;
struct FCodeAnalysisViewState;

struct FStaticAnalysisItem
{
	FStaticAnalysisItem(FAddressRef addr, const char* name):AddressRef(addr),Name(name){}

	FAddressRef		AddressRef;
	std::string		Name;

	virtual void	DrawUI(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState) const;
};

class FStaticAnalysisCheck
{
public:
	virtual ~FStaticAnalysisCheck(){}
	
	virtual void Reset(){}
	virtual FStaticAnalysisItem* RunCheck(FCodeAnalysisState& state, FAddressRef addrRef) = 0;
};

class FStaticAnalyser : public FViewerBase
{
public:
	FStaticAnalyser(FEmuBase* pEmu) : FViewerBase(pEmu) { Name = "Static Analysis"; }

	bool	Init(void) override;
	void	ResetForGame() override;
	void	Shutdown(void) override;
	void	DrawUI() override;

	//bool	Init(FCodeAnalysisState* pState);

	bool	RunAnalysis(void);
private:
	void	ResetAnalysis();

	std::vector<FStaticAnalysisCheck*>	Checks;

	std::vector<FStaticAnalysisItem*> Items;
};

// static analysis functions
EInstructionType GetInstructionType(FCodeAnalysisState& state, FAddressRef addr);