#pragma once
#include <cstdint>
#include <vector>
//#include <map>
#include <string>

#include <CodeAnalyser/CodeAnaysisPage.h> 

class FSpectrumEmu;
struct FGame;

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
	int		ExecCount[0x10000];
	int		ReadCount[0x10000];
	int		WriteCount[0x10000];

	std::vector< FMemoryBlock>	MemoryBlockInfo;

	std::vector<uint16_t>	CodeAndDataList;
};

enum class MemoryAccessType
{
	Read,
	Write,
	Execute
};

struct FMemoryAccessHandler
{
	// configuration
	std::string			Name;
	bool				bEnabled = true;
	bool				bBreak = false;
	MemoryAccessType	Type;
	uint16_t			MemStart;
	uint16_t			MemEnd;

	void(*pHandlerFunction)(FMemoryAccessHandler &handler, FGame* pGame, uint16_t pc, uint64_t pins) = nullptr;

	// stats
	int						TotalCount = 0;
	FItemReferenceTracker	Callers;
	//FItemReferenceTracker	AddressCounts;
};



int MemoryHandlerTrapFunction(uint16_t pc, int ticks, uint64_t pins, FSpectrumEmu* pEmu);

void AnalyseMemory(FMemoryStats &memStats);
void ResetMemoryStats(FMemoryStats &memStats);

// UI
void DrawMemoryAnalysis(FSpectrumEmu* pEmu);
void DrawMemoryHandlers(FSpectrumEmu* pEmu);
void DrawMemoryDiffUI(FSpectrumEmu* pEmu);