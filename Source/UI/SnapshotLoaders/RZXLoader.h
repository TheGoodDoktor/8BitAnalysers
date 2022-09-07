#pragma once

class FSpectrumEmu;

enum class EReplayMode
{
	Off,
	Playback,
	Record
};

class FRZXManager
{
public:
	void	Init(FSpectrumEmu* pEmu) { pZXEmulator = pEmu; }
	bool	Load(const char* fName);
	EReplayMode		GetReplayMode() const { return ReplayMode; }

private:
	FSpectrumEmu*	pZXEmulator = nullptr;
	EReplayMode		ReplayMode = EReplayMode::Off;

};

//bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName);
