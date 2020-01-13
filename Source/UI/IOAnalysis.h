#pragma once

#include <string>
#include <map>

struct FCodeAnalysisState;

enum class SpeccyIODevice
{
	None = -1,
	Keyboard,	
	Ear,	// for loading
	Mic,	// for saving
	Beeper,
	BorderColour,
	KempstonJoystick,

	Unknown,
	Count
};

struct FIOAccess
{
	//std::string		Name;
	uint16_t		Address = 0;
	int				ReadCount = 0;
	int				WriteCount = 0;

	std::map<uint16_t, int>	Callers;

};

struct FIOAnalysisState
{
	// io info
	//std::map<uint16_t, FIOAccess>		IOAccessMap;
	FIOAccess			IODeviceAcceses[(int)SpeccyIODevice::Count];
	uint8_t				LastFE = 0;
	SpeccyIODevice		SelectedDevice = SpeccyIODevice::None;
	FCodeAnalysisState*	pCodeAnalysis = nullptr;

};

void InitIOAnalysis(FIOAnalysisState &state);
void IOAnalysisHandler(FIOAnalysisState &state, uint16_t pc, uint64_t pins);
void DrawIOAnalysis(FIOAnalysisState &state);
