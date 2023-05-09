#pragma once

#include <string>
#include <CodeAnalyser/CodeAnalysisPage.h> 

class FSpectrumEmu;

enum class SpeccyIODevice
{
	None = -1,
	Keyboard,	
	Ear,	// for loading
	Mic,	// for saving
	Beeper,
	BorderColour,
	FloatingBus,
	KempstonJoystick,
	MemoryBank,		// Switching in/out RAM banks
	SoundChip,		// AY

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

	FItemReferenceTracker	Callers;
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
