#pragma once

#include "Util/GraphicsView.h"

class FCpcGraphicsView : public FGraphicsView
{
public:
	FCpcGraphicsView(int width, int height) :FGraphicsView(width, height) {}

	void DrawCharLine(uint8_t charLine, int xp, int yp, uint8_t colAttr = 0x7);
	void DrawBitImage(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint8_t colAttr = 0x7);
	void DrawBitImageChars(const uint8_t* pSrc, int xp, int yp, int widthChars, int heightChars, uint8_t colAttr = 0x7);

	uint32_t GetColFromAttr(uint8_t colBits, bool bBright);

private:

	static const uint32_t ColourLUT[32];
};