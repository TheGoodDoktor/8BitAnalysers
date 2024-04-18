#pragma once

#include <cinttypes>
#include <vector>
#include <map>

#include "CodeAnalyserTypes.h"
#include "FindTool.h"

class FCodeAnalysisState;

struct FPhysicalMemoryRange
{
	FPhysicalMemoryRange() = default;
	FPhysicalMemoryRange(uint16_t start, uint16_t end) :Start(start), End(end) {}
	bool		InRange(uint16_t addr) const { return addr >= Start && addr <= End; }

	uint16_t	Start = 0;
	uint16_t	End = 0;
};

// used for diffing memory banks
struct FBankMemory
{
	int16_t		BankId = -1;
	uint16_t	SizeBytes = 0;
	uint8_t* pMemory = nullptr;
};

class FMemoryAnalyser
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	Shutdown();
	void	FrameTick(void);
	void	DrawUI(void);

	void	ClearROMAreas(void) { ROMAreas.clear(); }
	void	AddROMArea(uint16_t start, uint16_t end) { ROMAreas.emplace_back(start,end); }

	void	SetScreenMemoryArea(uint16_t start, uint16_t end) { ScreenMemory = { start,end }; }
	bool	IsAddressInScreenMemory(uint16_t addr) const { return ScreenMemory.InRange(addr); }

	void FixupAddressRefs();

private:
	void	DrawMemoryDiffUI(void);
	void	DrawStringSearchUI(void);


private:
	FCodeAnalysisState*		pCodeAnalysis = nullptr;

	// Physical region identifiers
	std::vector<FPhysicalMemoryRange>	ROMAreas;
	FPhysicalMemoryRange	ScreenMemory;

	// Physical Memory Diff
	bool						bDiffPhysicalMemory = true;
	bool						bDiffVideoMem = false;
	bool						bSnapshotAvailable = false;
	std::map<int16_t,FBankMemory>	DiffSnapshotMemoryBanks;
	std::vector<FAddressRef>	DiffChangedLocations;

	FFindTool					FindTool;

	// String find
	bool						bSearchStringsInROM = true;
	bool						bSearchStringsPhysicalMemOnly = false;
	std::vector<FFoundString>	FoundStrings;
};