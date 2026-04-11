#include "MCPSuggestions.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/FunctionAnalyser.h"

void ApplySuggestion(FEmuBase* pEmu, const FMCPSuggestion& suggestion)
{
	FCodeAnalysisState& codeAnalysis = pEmu->GetCodeAnalysis();
	const FAddressRef addrRef = suggestion.AddressRef;

	switch (suggestion.Type)
	{
	case EMCPSuggestionType::RenameFunction:
	case EMCPSuggestionType::SetLabel:
	{
		FLabelInfo* pLabel = codeAnalysis.GetLabelForAddress(addrRef);
		if (pLabel)
		{
			pLabel->ChangeName(suggestion.NewValue.c_str(), addrRef);
		}
		else
		{
			FLabelInfo* pNewLabel = FLabelInfo::Allocate();
			pNewLabel->InitialiseName(suggestion.NewValue.c_str());
			pNewLabel->LabelType = ELabelType::Code;
			pNewLabel->Global = true;
			pNewLabel->EnsureUniqueName(addrRef);
			codeAnalysis.SetLabelForAddress(addrRef, pNewLabel);
		}
		codeAnalysis.SetCodeAnalysisDirty(addrRef);
		break;
	}

	case EMCPSuggestionType::SetFunctionDescription:
	{
		FFunctionInfo* pFuncInfo = codeAnalysis.pFunctions->FindFunctionByName(suggestion.TargetName.c_str());
		if (pFuncInfo)
		{
			pFuncInfo->Description = suggestion.NewValue;
			codeAnalysis.SetCodeAnalysisDirty(pFuncInfo->StartAddress);
		}
		break;
	}

	case EMCPSuggestionType::AddComment:
	{
		FCodeInfo* pCodeInfo = codeAnalysis.GetCodeInfoForAddress(addrRef);
		if (pCodeInfo)
			pCodeInfo->Comment = suggestion.NewValue;
		break;
	}
	}
}

void FMCPSuggestionQueue::Add(const FMCPSuggestion& suggestion)
{
	std::lock_guard<std::mutex> lock(Mutex);
	Suggestions.push_back(suggestion);
}

void FMCPSuggestionQueue::Remove(int index)
{
	std::lock_guard<std::mutex> lock(Mutex);
	if (index >= 0 && index < (int)Suggestions.size())
		Suggestions.erase(Suggestions.begin() + index);
}

void FMCPSuggestionQueue::Clear()
{
	std::lock_guard<std::mutex> lock(Mutex);
	Suggestions.clear();
}

std::vector<FMCPSuggestion> FMCPSuggestionQueue::GetAll() const
{
	std::lock_guard<std::mutex> lock(Mutex);
	return Suggestions;
}

int FMCPSuggestionQueue::Count() const
{
	std::lock_guard<std::mutex> lock(Mutex);
	return (int)Suggestions.size();
}
