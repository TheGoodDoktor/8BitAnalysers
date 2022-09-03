#pragma once

#include <cstdint>
#include <vector>

class FSpectrumEmu;

struct FSpeccyFrameTrace
{
	void*					Texture;
	std::vector<uint16_t>	InstructionTrace;
};

class FFrameTraceViewer
{
public:
	void	Init(FSpectrumEmu* pEmu);
	void	CaptureFrame();
	void	Draw();
private:

	FSpectrumEmu* pSpectrumEmu = nullptr;

	int					ShowFrame = 0;
	int					CurrentTraceFrame = 0;
	static const int	kNoFramesInTrace = 300;
	FSpeccyFrameTrace	FrameTrace[kNoFramesInTrace];

};