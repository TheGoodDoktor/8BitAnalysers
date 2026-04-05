
#pragma once
#include <string>
#include <map>
#include <vector>

class FPCEEmu;
struct FPerfStats;

struct FGameDebugStats
{
	// remove? it exists in gamedb now.
	int NumDupeBanks = 0;
	int NumBanks = 0;
	int NumBanksMapped = 0;
	int MaxBankSwitches = 0;
	
	float AvgFrameRate = 0.f;
};

struct FEmuDebugStats
{
	FEmuDebugStats();

	void Update(float deltaTime);
	void Reset();
	void InitForGame(FPCEEmu* pEmu, const std::string& gameName);
	FGameDebugStats* GetDebugStatsForGame(const std::string& gameName);

	// Debug stats for each game. Uses project name as key
	std::map<std::string, FGameDebugStats> GameDebugStats;

	int NumBankSwitchesThisFrame = 0;

	double GetAverageFrameRate() const;

protected:
	FPerfStats* pPerfStats = nullptr;
};
