#pragma once

#include <string>

class FSpectrumEmu;

enum class EReplayMode
{
	Off,
	Playback,
	Record
};

struct FRZXData;

class FRZXManager
{
public:
	bool			Init(FSpectrumEmu* pEmu);
	bool			Load(const char* fName);
	void			DrawUI(void);
	//void			RegisterInstructions(int num);
	uint32_t		Update();
	bool			GetInput(uint8_t& outVal);
	EReplayMode		GetReplayMode() const { return ReplayMode; }
	//bool			RZXCallbackHandler(int msg, void* param);
private:

	FSpectrumEmu*	pZXEmulator = nullptr;
	bool			Initialised = false;
	EReplayMode		ReplayMode = EReplayMode::Off;
	int				FrameNo = 0;
	int				InputCount = 0;

	FRZXData*	pData = nullptr;
};

//bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName);
