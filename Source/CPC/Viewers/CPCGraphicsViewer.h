#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <CodeAnalyser/UI/GraphicsViewer.h>

class FCPCEmu;
struct FGame;

// test
class FCPCGraphicsView;

class FCPCGraphicsViewer : public FGraphicsViewer
{
public:
	FCPCGraphicsViewer()
	{
		ScreenWidth = 320;
		ScreenHeight = 200;
	}

	void	Draw() override;
	void	DrawScreenViewer(void) override;
	void	Init(FCodeAnalysisState* pCodeAnalysis, FCPCEmu* pEmu);

	void	DrawPaletteViewer();
	void	DrawPalette(const uint32_t* palette, int numColours);
protected:
	const uint32_t* GetCurrentPalette() const override;

private:
	uint32_t	GetRGBValueForPixel(int yPos, int colourIndex, uint32_t heatMapCol) const;
	void		UpdateScreenPixelImage(void);
	uint16_t	GetPixelLineOffset(int yPos);

	FCPCEmu*	pCPCEmu = nullptr;

	int			DisplayAddress = 0xc000;
	int			WidthChars = 40;
	int			HeightChars = 25;
	int			ScreenMode = 1;
	int			CharacterHeight = 8;

#if 0
	FGraphicsView* pTestGraphicsView = 0;
	FCPCGraphicsView* pTestCPCGraphicsView = 0;
#endif
};
