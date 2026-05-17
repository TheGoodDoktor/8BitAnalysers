#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>
#include "CodeAnalyser/CodeAnalyserTypes.h"

enum class EMCPSuggestionType
{
	RenameFunction,
	SetFunctionDescription,
	SetLabel,
	AddComment,
};

struct FMCPSuggestion
{
	EMCPSuggestionType	Type;
	FAddressRef			AddressRef;			// reference to the address being modified (for applying the change)
	//uint16_t			Address = 0;		// target address
	std::string			TargetName;			// function/label being targeted (for display)
	std::string			OldValue;			// current value, shown in UI for comparison
	std::string			NewValue;			// proposed new value
	std::string			Rationale;			// LLM's reasoning, shown in UI
};

class FEmuBase;

void ApplySuggestion(FEmuBase* pEmu, const FMCPSuggestion& suggestion);

class FMCPSuggestionQueue
{
public:
	void Add(const FMCPSuggestion& suggestion);
	void Remove(int index);
	void Clear();

	// Returns a snapshot copy so the UI doesn't need to hold the lock while drawing
	std::vector<FMCPSuggestion> GetAll() const;
	int Count() const;

	void Save(const std::string& filePath) const;
	void Load(const std::string& filePath);

private:
	mutable std::mutex			Mutex;
	std::vector<FMCPSuggestion>	Suggestions;
};
