#pragma once

#include <string>
#include <map>

struct FCodeAnalysisState;

enum class SpeccyIODevice
{
	None = -1,
	Keyboard,	//TODO: rows
	Beeper,

	Unknown,
	Count
};

struct FIOAccess
{
	std::string		Name;
	uint16_t		Address;
	int				ReadCount = 0;
	int				WriteCount = 0;

	std::map<uint16_t, int>	Callers;

};

struct FIOAnalysisState
{
	// io info
	std::map<uint16_t, FIOAccess>		IOAccessMap;
	FIOAccess							IODeviceAcceses[(int)SpeccyIODevice::Count];

	FCodeAnalysisState*	pCodeAnalysis = nullptr;

};

void InitIOAnalysis(FIOAnalysisState &state);
void IOAnalysisHandler(FIOAnalysisState &state, uint16_t pc, uint64_t pins);
void DrawIOAnalysis(FIOAnalysisState &state);
