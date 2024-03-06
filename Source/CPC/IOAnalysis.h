#pragma once

#include <string>
#include <CodeAnalyser/CodeAnalysisPage.h> 

class FCPCEmu;

enum class EEventType : uint8_t
{
	None = 0,
	ScreenPixWrite,
	PaletteSelect,
	PaletteColour,
	BorderColour,
	ScreenModeChange,
	CrtcRegisterSelect,
	CrtcRegisterRead,
	CrtcRegisterWrite,
	KeyboardRead,
	// this is technically the same event as setting CRTC registers R12 or R13 but we have a separate event for convenience.
	ScreenMemoryAddressChange,
	//SoundChipRead,
	//SoundChipRegisterSelect,
	//SoundChipRegisterWrite,
	SwitchMemoryBanks,
};

enum class CPCIODevice
{
	None = -1,
	Keyboard,
	Joystick,
	CRTC,
	PaletteRegisterSelect,
	PaletteWrite,
	BorderColour,

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

	FItemReferenceTracker	Readers;
	FItemReferenceTracker	Writers;
};

class FIOAnalysis
{
public:
  void	Init(FCPCEmu* pEmu);
  void	IOHandler(uint16_t pc, uint64_t pins);
  void	DrawUI();
  void	Reset();

private:
  void HandlePPI(uint64_t pins, CPCIODevice& readDevice, CPCIODevice& writeDevice);
  void HandleCRTC(uint64_t pins, CPCIODevice& readDevice, CPCIODevice& writeDevice);
  void HandleGateArray(uint64_t pins, CPCIODevice& readDevice, CPCIODevice& writeDevice);
  void RegisterEvent(uint8_t type, uint16_t address, uint8_t value);

  FCPCEmu*	  pCPCEmu = nullptr;
  FIOAccess	  IODeviceAcceses[(int)CPCIODevice::Count];
  uint8_t	  LastFE = 0;
  CPCIODevice SelectedDevice = CPCIODevice::None;
  int		  CurScreenMode = -1;
};
