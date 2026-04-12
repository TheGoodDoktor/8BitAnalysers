#include "MCPSuggestions.h"
#include "Misc/EmuBase.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/FunctionAnalyser.h"
#include "json.hpp"
#include <fstream>

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

	// Reject duplicates: same type targeting the same address (or same name if no address)
	for (const auto& existing : Suggestions)
	{
		if (existing.Type != suggestion.Type)
			continue;

		if (suggestion.AddressRef.IsValid() && existing.AddressRef.IsValid())
		{
			if (existing.AddressRef == suggestion.AddressRef)
				return;
		}
		else
		{
			if (existing.TargetName == suggestion.TargetName)
				return;
		}
	}

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

void FMCPSuggestionQueue::Save(const std::string& filePath) const
{
	std::lock_guard<std::mutex> lock(Mutex);

	nlohmann::json jsonArray = nlohmann::json::array();
	for (const auto& s : Suggestions)
	{
		nlohmann::json item;
		item["type"]       = (int)s.Type;
		item["bankId"]     = s.AddressRef.BankId;
		item["address"]    = s.AddressRef.Address;
		item["targetName"] = s.TargetName;
		item["oldValue"]   = s.OldValue;
		item["newValue"]   = s.NewValue;
		item["rationale"]  = s.Rationale;
		jsonArray.push_back(item);
	}

	std::ofstream file(filePath);
	if (file.is_open())
		file << jsonArray.dump(2);
}

void FMCPSuggestionQueue::Load(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		return;

	nlohmann::json jsonArray;
	try
	{
		file >> jsonArray;
	}
	catch (...) { return; }

	if (!jsonArray.is_array())
		return;

	std::lock_guard<std::mutex> lock(Mutex);
	Suggestions.clear();

	for (const auto& item : jsonArray)
	{
		try
		{
			FMCPSuggestion s;
			s.Type        = (EMCPSuggestionType)item["type"].get<int>();
			s.AddressRef  = FAddressRef(item["bankId"].get<int16_t>(), item["address"].get<uint16_t>());
			s.TargetName  = item["targetName"].get<std::string>();
			s.OldValue    = item["oldValue"].get<std::string>();
			s.NewValue    = item["newValue"].get<std::string>();
			s.Rationale   = item["rationale"].get<std::string>();
			Suggestions.push_back(s);
		}
		catch (...) { /* skip malformed entries */ }
	}
}
