#pragma once

#include <cstdint>
#include <vector>

struct FSpeccyUI;

struct FSpeccyFrameTrace
{
	void*					Texture;
	std::vector<uint16_t>	InstructionTrace;
};

class FFrameTraceViewer
{
public:
	void	Init(FSpeccyUI* pEmu);
	void	CaptureFrame();
	void	Draw();
private:

	FSpeccyUI* pSpectrumEmu = nullptr;

	int					ShowFrame = 0;
	int					CurrentTraceFrame = 0;
	static const int	kNoFramesInTrace = 300;
	FSpeccyFrameTrace	FrameTrace[kNoFramesInTrace];

};