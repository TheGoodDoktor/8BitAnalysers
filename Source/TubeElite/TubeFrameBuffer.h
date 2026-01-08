#pragma once

#include <cinttypes>

namespace Display
{
	void Init(void);
	int GetFrameCount(void);
	void WaitVBlank(void);

	void SetPalette(uint8_t colour, uint8_t r, uint8_t g, uint8_t b);

	// drawing functions
	void ClearScreen(uint8_t colour = 0); // Clear the screen to a specific color (default is black)
	void ClearScreenFromYpos(int y, uint8_t col = 0);
	void DrawPixel(uint16_t x, uint16_t y, uint8_t colour);
	void DrawPixelEOR(uint16_t x, uint16_t y, uint8_t colour);
	void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colour);
	void DrawLineEOR(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colour);
	void DrawHLineEOR(uint16_t x1, uint16_t x2, uint16_t y, uint8_t colour);
	void DrawChar8x8(uint16_t x, uint16_t y, uint8_t ch, uint8_t colour);


	void RenderFrame();
}