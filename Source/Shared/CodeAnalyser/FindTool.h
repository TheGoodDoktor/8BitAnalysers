#pragma once

#include "CodeAnalyserTypes.h"

#include <cinttypes>
#include <vector>

#include "Util/Misc.h"

class FCodeAnalysisState;

enum ESearchMemoryType
{
	SearchData = 0,		// search data only
	SearchCode,			// search code only
	SearchCodeAndData,	// search code and data
};

struct FSearchOptions
{
	ESearchMemoryType MemoryType = ESearchMemoryType::SearchCodeAndData; // What type of memory locations to search?
	bool bSearchUnreferenced = true; // Search locations with no references
	bool bSearchUnaccessed = true;	 // Search locations that have not been written or read.
	bool bSearchGraphicsMem = false; // Include graphics memory in the search?
	bool bSearchPhysicalOnly = false;// If true, will search only physical memory. If false, will search the all memory banks, including ones paged out.
	bool bSearchROM = false;		 // Include ROM in the search?
};

class FFinder
{
public:
	void Init(FCodeAnalysisState* ptrCodeAnalysis);
	
	virtual void Find(const FSearchOptions& opt);
	virtual void Reset();
	virtual void ProcessMatch(FAddressRef addr, const FSearchOptions& opt);

	virtual bool HasValueChanged(FAddressRef addr) const;
	virtual const char* GetValueString(FAddressRef addr, ENumberDisplayMode numberMode) const = 0;
	virtual void RemoveUnchangedResults();
	
	size_t GetNumResults() const { return SearchResults.size(); }
	FAddressRef GetResult(size_t index) const { return SearchResults[index]; }

protected:
	virtual std::vector<FAddressRef> FindAllMatchesInBanks(const FSearchOptions& opt) = 0;

protected:
	std::vector<FAddressRef> SearchResults;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
};

class FDataFinder : public FFinder
{
};

class FByteFinder : public FDataFinder
{
public:
	virtual bool HasValueChanged(FAddressRef addr) const override;
	virtual void Find(const FSearchOptions& opt) override
	{
		LastValue = SearchValue;
		FFinder::Find(opt);
	}
	virtual std::vector<FAddressRef> FindAllMatchesInBanks(const FSearchOptions& opt) override;
	virtual const char* GetValueString(FAddressRef addr, ENumberDisplayMode numberMode) const override;
	uint8_t SearchValue = 0;
	uint8_t LastValue = 0;
};

class FWordFinder : public FDataFinder
{
public:
	virtual bool HasValueChanged(FAddressRef addr) const override;
	virtual void Find(const FSearchOptions& opt) override;
	virtual std::vector<FAddressRef> FindAllMatchesInBanks(const FSearchOptions& opt) override;
	virtual const char* GetValueString(FAddressRef addr, ENumberDisplayMode numberMode) const override;
	uint16_t SearchValue = 0;
	uint8_t SearchBytes[2];
	uint16_t LastValue = 0;
};

class FTextFinder : public FFinder
{
public:
	virtual std::vector<FAddressRef> FindAllMatchesInBanks(const FSearchOptions& opt) override;
	virtual const char* GetValueString(FAddressRef addr, ENumberDisplayMode numberMode) const override { return ""; }
	std::string SearchText;
};

enum ESearchType
{
	SearchValue,
	SearchText,
};

enum ESearchDataType
{
	SearchByte = 0,
	SearchWord,
};


class FFindTool
{
public:
	FFindTool();
	void Init(FCodeAnalysisState* ptrCodeAnalysis);
	void DrawUI();
	void Reset();

private:
	FSearchOptions Options;
	ESearchType SearchType = ESearchType::SearchValue;
	ESearchDataType DataSize = ESearchDataType::SearchByte;

	bool bDecimal = true;

	FFinder* pCurFinder = nullptr;

	FByteFinder ByteFinder;
	FWordFinder WordFinder;
	FTextFinder TextFinder;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
};
