#pragma once

#include <string>
#include <CodeAnalyser/CodeAnalysisPage.h> 

class FCpcEmu;

enum class CpcIODevice
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
  void	Init(FCpcEmu* pEmu);
  void	IOHandler(uint16_t pc, uint64_t pins);
  void	DrawUI();
  void	Reset();

private:
  void HandlePPI(uint64_t pins, CpcIODevice& readDevice, CpcIODevice& writeDevice);
  void HandleCRTC(uint64_t pins, CpcIODevice& readDevice, CpcIODevice& writeDevice);
  void HandleGateArray(uint64_t pins, CpcIODevice& readDevice, CpcIODevice& writeDevice);
  void RegisterEvent(uint8_t type, uint16_t address, uint8_t value);

  FCpcEmu*	  pCpcEmu = nullptr;
  FIOAccess	  IODeviceAcceses[(int)CpcIODevice::Count];
  uint8_t	  LastFE = 0;
  CpcIODevice SelectedDevice = CpcIODevice::None;
  int		  CurScreenMode = -1;
};
