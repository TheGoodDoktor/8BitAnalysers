#pragma once

#include <cstdint>

#include <CodeAnalyser/CodeAnalyserTypes.h>
#include <CodeAnalyser/UI/GraphicsViewer.h>
#include "imgui.h"

class FCPCEmu;
struct FGame;

// test
//class FCPCGraphicsView;

class FCPCGraphicsViewer : public FGraphicsViewer
{
public:
	FCPCGraphicsViewer(FEmuBase *pEmu) : FGraphicsViewer(pEmu)
	{
		ScreenWidth = 320;
		ScreenHeight = 200;
	}

	void	DrawUI() override;
	void	DrawScreenViewer(void) override;
	bool	Init(void) override;

	void	DrawPaletteViewer();

	void	OnScreenAddressChanged(uint16_t addr);

protected:
	const uint32_t* GetCurrentPalette() const override;

private:
	uint32_t	GetRGBValueForPixel(int yPos, int colourIndex, uint32_t heatMapCol) const;
	void		UpdateScreenPixelImage(void);
	uint16_t	GetPixelLineOffset(int yPos);
	ImU32		GetFlashColour() const;

	FCPCEmu*	pCPCEmu = nullptr;

	int			DisplayAddress = 0xc000;
	int			WidthChars = 40;
	int			HeightChars = 25;
	int			ScreenMode = 1;
	int			CharacterHeight = 8;
	bool			bShowReadsWrites = true;
	int			PaletteNo = -1;
	uint32_t*	pPaletteColours = nullptr;
	uint16_t		ScrAddrHistory[2] = { 0xc000, 0xc000 };
	int			ScrAddrHistoryIndex = 0;
	int			FrameCounter = 0;
	int			TestHeatmapThreshold = 4;
	bool			bTrackNonActiveScrBufferAddress = false;
#if 0
	FGraphicsView* pTestGraphicsView = 0;
	FCPCGraphicsView* pTestCPCGraphicsView = 0;
#endif
};
