#pragma once

#include <cinttypes>
#include <vector>

#include "CodeAnalyserTypes.h"

class FCodeAnalysisState;

struct FPhysicalMemoryRange
{
	FPhysicalMemoryRange() = default;
	FPhysicalMemoryRange(uint16_t start, uint16_t end) :Start(start), End(end) {}
	bool		InRange(uint16_t addr) const { return addr >= Start && addr <= End; }

	uint16_t	Start = 0;
	uint16_t	End = 0;
};

class FMemoryAnalyser
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	FrameTick(void);
	void	DrawUI(void);

	void	SetROMArea(uint16_t start, uint16_t end) { ROMArea = { start,end }; }

	void	SetScreenMemoryArea(uint16_t start, uint16_t end) { ScreenMemory = { start,end }; }
	bool	IsAddressInScreenMemory(uint16_t addr) const { return ScreenMemory.InRange(addr); }
private:
	void	DrawPhysicalMemoryDiffUI(void);


private:
	FCodeAnalysisState*		pCodeAnalysis = nullptr;

	// Physical region identifiers
	FPhysicalMemoryRange	ROMArea;
	FPhysicalMemoryRange	ScreenMemory;

	// Physical Memory Diff
	bool						bDiffVideoMem = false;
	bool						bSnapshotAvailable = false;
	uint8_t						DiffSnapShotMemory[1 << 16];	// 64 Kb
	std::vector<FAddressRef>	DiffChangedLocations;


};