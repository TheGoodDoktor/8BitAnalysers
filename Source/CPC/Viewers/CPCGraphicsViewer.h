#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <CodeAnalyser/UI/GraphicsViewer.h>

class FCpcEmu;
struct FGame;
class FCpcEmu;

class FCPCGraphicsViewer : public FGraphicsViewer
{
public:
	FCPCGraphicsViewer()
	{
		ScreenWidth = 320;
		ScreenHeight = 200;
	}

	void	DrawScreenViewer(void) override;
	void	Init(FCodeAnalysisState* pCodeAnalysis, FCpcEmu* pEmu);

private:
	uint32_t	GetRGBValueForPixel(int yPos, int colourIndex, uint32_t heatMapCol) const;
	void		UpdateScreenPixelImage(void);
	uint16_t	GetPixelLineOffset(int yPos);

	FCpcEmu*	pCpcEmu = nullptr;

	int			DisplayAddress = 0xc000;
	int			WidthChars = 40;
	int			HeightChars = 25;
	int			ScreenMode = 1;
	int			CharacterHeight = 8;
};
