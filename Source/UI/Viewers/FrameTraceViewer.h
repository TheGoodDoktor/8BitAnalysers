#pragma once


#include "CodeAnalyser/CodeAnalyser.h"

#include <cstdint>
#include <vector>
#include <string>

class FSpectrumEmu;
struct FGraphicsView;

struct FFrameOverviewItem
{
	std::string		Label;
	uint16_t		FunctionAddress;
	uint16_t		LabelAddress;
};
struct FSpeccyFrameTrace
{
	void*					Texture;
	std::vector<uint16_t>	InstructionTrace;
	std::vector<FMemoryAccess>	ScreenPixWrites;
	std::vector<FMemoryAccess>	ScreenAttrWrites;

	std::vector< FFrameOverviewItem>	FrameOverview;
};

class FFrameTraceViewer
{
public:
	void	Init(FSpectrumEmu* pEmu);
	void	Shutdown();
	void	CaptureFrame();
	void	Draw();
private:
	void	DrawInstructionTrace(const FSpeccyFrameTrace& frame);
	void	GenerateTraceOverview(FSpeccyFrameTrace& frame);
	void	DrawTraceOverview(const FSpeccyFrameTrace& frame);
	void	DrawFrameScreenWritePixels(const FSpeccyFrameTrace& frame, int lastIndex = -1);
	void	DrawScreenWrites(const FSpeccyFrameTrace& frame);

	FSpectrumEmu* pSpectrumEmu = nullptr;

	int					ShowFrame = 0;
	int					CurrentTraceFrame = 0;
	static const int	kNoFramesInTrace = 300;
	FSpeccyFrameTrace	FrameTrace[kNoFramesInTrace];

	int		PixelWriteline = -1;
	FGraphicsView*	ShowWritesView = nullptr;

};