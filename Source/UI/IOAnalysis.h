#pragma once

#include <string>
#include <map>

class FSpectrumEmu;

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
	uint16_t		Address = 0;
	int				ReadCount = 0;
	int				FrameReadCount = 0;
	int				WriteCount = 0;
	int				FrameWriteCount = 0;

	std::map<uint16_t, int>	Callers;
};

class FIOAnalysis
{
public:
	void	Init(FSpectrumEmu* pEmu);
	void	IOHandler(uint16_t pc, uint64_t pins);
	void	DrawUI();

private:
	FSpectrumEmu*		pSpectrumEmu = nullptr;
	FIOAccess			IODeviceAcceses[(int)SpeccyIODevice::Count];
	uint8_t				LastFE = 0;
	SpeccyIODevice		SelectedDevice = SpeccyIODevice::None;
};
