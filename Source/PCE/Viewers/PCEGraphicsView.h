#pragma once

#include "Util/GraphicsView.h"

class FPCEEmu;

class FPCEGraphicsView : public FGraphicsView
{
public:
	FPCEGraphicsView(FPCEEmu* pEmu, int width, int height) 
		: FGraphicsView(width, height)
		, pPCEEmu(pEmu)
	{}

	// Height and width are in 16x16 sprite character blocks.
	// PaletteIndex is sprite palette index 0-15
	void Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int width, int height, int paletteIndex = 0);

	// Height and width are in 8x8 BG tile blocks.
	// PaletteIndex is BG palette index 0-15
	void Draw4bppBGTileImage(const uint8_t* pSrc, int xp, int yp, int width, int height, int paletteIndex = 0);

protected:
	FPCEEmu* pPCEEmu = nullptr;
};