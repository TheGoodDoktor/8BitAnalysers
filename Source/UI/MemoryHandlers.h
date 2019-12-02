#pragma once
#include <cstdint>
#include <vector>

struct FSpeccyUI;

enum class MemoryUse
{
	Code,
	Data,
	Unknown	// unknown/unused - never read from or written to
};

struct FMemoryBlock
{
	MemoryUse	Use;
	uint16_t	StartAddress;
	uint16_t	EndAddress;
	bool		ContainsSelfModifyingCode = false;
};

struct FMemoryStats
{
	// counters for each memory address
	int		ExecCount[0xffff];
	int		ReadCount[0xffff];
	int		WriteCount[0xffff];

	std::vector< FMemoryBlock>	MemoryBlockInfo;

	std::vector<uint16_t>	CodeAndDataList;
};


int MemoryHandlerTrapFunction(uint16_t pc, int ticks, uint64_t pins, FSpeccyUI *pUI);

void AnalyseMemory(FMemoryStats &memStats);
void ResetMemoryStats(FMemoryStats &memStats);

// UI
void DrawMemoryAnalysis(FSpeccyUI* pUI);
void DrawMemoryHandlers(FSpeccyUI* pUI);
