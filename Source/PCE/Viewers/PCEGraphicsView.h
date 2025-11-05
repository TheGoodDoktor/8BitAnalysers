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

	void Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex = 0);
	//void DrawMode0Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex = 0);
	//void DrawMode1Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex = 0);
protected:
	FPCEEmu* pPCEEmu = nullptr;
};