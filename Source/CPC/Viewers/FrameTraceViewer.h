#pragma once


#include "CodeAnalyser/CodeAnalyser.h"

#include <cstdint>
#include <vector>
#include <string>

class FCPCEmu;
class FCPCGraphicsView;

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
	//SwitchMemoryBanks,
};

struct FFrameOverviewItem
{
	std::string		Label;
	uint16_t		FunctionAddress;
	uint16_t		LabelAddress;
};

struct FMemoryDiff
{
	int			Bank;
	uint16_t	Address;
	uint8_t		OldVal;
	uint8_t		NewVal;
};

struct FCPCFrameTrace
{
	void*					Texture = nullptr;
	uint8_t					MemoryBanks[8][16 * 1024];	// 8 x 16K banks
	uint8_t					MemoryBankRegister = 0;
	void*					CPUState = nullptr;
	std::vector<FAddressRef>	InstructionTrace;
	std::vector<FMemoryAccess>	ScreenPixWrites;
	std::vector<FEvent>			FrameEvents;

	std::vector<FFrameOverviewItem>	FrameOverview;
	std::vector<FMemoryDiff>	MemoryDiffs;
};

class FFrameTraceViewer
{
public:
	void	Init(FCPCEmu* pEmu);
	void	Reset();
	void	Shutdown();
	void	CaptureFrame();
	void	Draw();
private:
	void	RestoreFrame(const FCPCFrameTrace& frame);
	void	DrawInstructionTrace(const FCPCFrameTrace& frame);
	void	GenerateTraceOverview(FCPCFrameTrace& frame);
	void	GenerateMemoryDiff(const FCPCFrameTrace& frameA, const FCPCFrameTrace& frameB, std::vector<FMemoryDiff>& outDiff);
	void	DrawTraceOverview(const FCPCFrameTrace& frame);
	void	DrawFrameScreenWritePixels(const FCPCFrameTrace& frame, int lastIndex = -1);
	void	DrawScreenWrites(const FCPCFrameTrace& frame);
	void	DrawMemoryDiffs(const FCPCFrameTrace& frame);

	FCPCEmu* pCPCEmu = nullptr;

	int					ShowFrame = 0;
	int					CurrentTraceFrame = 0;
	bool				RestoreOnScrub = false;
	static const int	kNoFramesInTrace = 300;
	FCPCFrameTrace	FrameTrace[kNoFramesInTrace];

	int		SelectedTraceLine = -1;
	int		PixelWriteline = -1;
	FCPCGraphicsView*	ShowWritesView = nullptr;

};