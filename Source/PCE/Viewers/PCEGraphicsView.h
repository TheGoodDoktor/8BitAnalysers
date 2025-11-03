#pragma once

#include "Util/GraphicsView.h"

class FPCEGraphicsView : public FGraphicsView
{
public:
	FPCEGraphicsView(int width, int height) :FGraphicsView(width, height) {}

	//void DrawMode0Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex = 0);
	//void DrawMode1Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex = 0);
};