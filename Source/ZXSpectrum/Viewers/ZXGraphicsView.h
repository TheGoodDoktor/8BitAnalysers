#pragma once

#include "Util/GraphicsView.h"

class FZXGraphicsView : public FGraphicsView
{
public:
	FZXGraphicsView(int width, int height) :FGraphicsView(width, height) {}

	void DrawCharLine(uint8_t charLine, int xp, int yp, uint8_t colAttr = 0x7);
	void DrawBitImage(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint8_t colAttr = 0x7);
	void DrawBitImageChars(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint8_t colAttr = 0x7);

	uint32_t GetColFromAttr(uint8_t colBits, bool bBright);

	static const uint32_t* GetColourLUT() { return ColourLUT; }

private:

	static const uint32_t ColourLUT[8];
};