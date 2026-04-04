#include "DebugStats.h"
#include "PCEEmu.h"

struct FPerfStats
{
	double TotalTimeSeconds = 0.0;
	uint64_t TotalFrames = 0;

	void Reset()
	{
		TotalTimeSeconds = 0.0;
		TotalFrames = 0;
	}

	void Update(float deltaTime)
	{
		TotalTimeSeconds += deltaTime;
		TotalFrames++;
	}

	double GetAverageFPS() const
	{
		return TotalTimeSeconds > 0.0 ? (double)TotalFrames / TotalTimeSeconds : 0.0;
	}
};

FEmuDebugStats::FEmuDebugStats()
{
	pPerfStats = new FPerfStats;
}

FGameDebugStats* FEmuDebugStats::GetDebugStatsForGame(const std::string& gameName)
{
	auto it = GameDebugStats.find(gameName);
	if (it != GameDebugStats.end())
	{
		return &it->second;
	}
	return nullptr;
}

void FEmuDebugStats::InitForGame(FPCEEmu* pEmu, const std::string& gameName)
{
	GameDebugStats[gameName].NumBanks = pEmu->GetBankCount();
	pPerfStats->Reset();
}

void FEmuDebugStats::Reset()
{
	GameDebugStats.clear();
}

void FEmuDebugStats::Update(float deltaTime)
{
	pPerfStats->Update(deltaTime);
}

double FEmuDebugStats::GetAverageFrameRate() const
{
	return pPerfStats->GetAverageFPS();
}
