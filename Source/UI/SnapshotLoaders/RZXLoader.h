#pragma once

#include <string>

class FSpectrumEmu;

enum class EReplayMode
{
	Off,
	Playback,
	Record
};

struct FRZXInfo
{
	std::string		Name;

};

class FRZXManager
{
public:
	bool			Init(FSpectrumEmu* pEmu);
	bool			Load(const char* fName);
	void			Update();
	uint8_t			GetInput();
	EReplayMode		GetReplayMode() const { return ReplayMode; }
	bool			RZXCallbackHandler(int msg, void* param);
private:
	FSpectrumEmu*	pZXEmulator = nullptr;
	bool			Initialised = false;
	EReplayMode		ReplayMode = EReplayMode::Off;
	FRZXInfo		CurrentRZXInfo;

};

//bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName);
