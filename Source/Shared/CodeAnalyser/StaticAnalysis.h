#pragma once

#include <vector>
#include <string>
#include "CodeAnalyserTypes.h"

enum class EInstructionType
{
	AddToSelf,
	LoopBack,
	PortInput,
	PortOutput,

	Unknown,
};

class FCodeAnalysisState;
struct FCodeAnalysisViewState;

struct FStaticAnalysisItem
{
	FStaticAnalysisItem(FAddressRef addr, const char* name):AddressRef(addr),Name(name){}

	FAddressRef		AddressRef;
	std::string		Name;

	virtual void	DrawUi(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState);
};

class FStaticAnalysisCheck
{
public:
	virtual void Reset(){}
	virtual FStaticAnalysisItem* RunCheck(FCodeAnalysisState& state, FAddressRef addrRef) = 0;
};

class FStaticAnalyser
{
public:
	bool	Init(FCodeAnalysisState* pState);

	void	Reset();
	bool	RunAnalysis(void);

	void	DrawUI(void);
private:

	FCodeAnalysisState*	pCodeAnalysis = nullptr;

	std::vector<FStaticAnalysisCheck*>	Checks;

	std::vector<FStaticAnalysisItem*> Items;
};