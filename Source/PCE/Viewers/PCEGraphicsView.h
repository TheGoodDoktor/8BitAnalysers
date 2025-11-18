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

	void Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int width, int height, const uint32_t* cols);
	void Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int width, int height, int paletteIndex = 0);
protected:
	FPCEEmu* pPCEEmu = nullptr;
};