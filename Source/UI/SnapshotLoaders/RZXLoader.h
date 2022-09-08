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
	std::string		Creator;

};

class FRZXManager
{
public:
	bool			Init(FSpectrumEmu* pEmu);
	bool			Load(const char* fName);
	void			DrawUI(void);
	uint16_t		Update();
	uint8_t			GetInput();
	EReplayMode		GetReplayMode() const { return ReplayMode; }
	bool			RZXCallbackHandler(int msg, void* param);
private:
	FSpectrumEmu*	pZXEmulator = nullptr;
	bool			Initialised = false;
	EReplayMode		ReplayMode = EReplayMode::Off;
	FRZXInfo		CurrentRZXInfo;

	int				IRBTStates = 0;
	uint16_t		ICount = 0;
	uint8_t			LastInput = 0;
	int				InputsThisFrame = 0;
	int				LastFrameInputVals = 0;
	int				LastFrameInputCalls = 0;


};

//bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName);
