#pragma once

#include <cstdint>
#include <vector>
#include <string>

class FSpectrumEmu;

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
	std::vector< FFrameOverviewItem>	FrameOverview;
};

class FFrameTraceViewer
{
public:
	void	Init(FSpectrumEmu* pEmu);
	void	CaptureFrame();
	void	Draw();
private:
	void	DrawInstructionTrace(const FSpeccyFrameTrace& frame);
	void	GenerateTraceOverview(FSpeccyFrameTrace& frame);
	void	DrawTraceOverview(const FSpeccyFrameTrace& frame);

	FSpectrumEmu* pSpectrumEmu = nullptr;

	int					ShowFrame = 0;
	int					CurrentTraceFrame = 0;
	static const int	kNoFramesInTrace = 300;
	FSpeccyFrameTrace	FrameTrace[kNoFramesInTrace];

};