#include "TubeFrameBuffer.h"
#include "Util/GraphicsView.h"
#include "BBCFont.h"
#include "Debug/DebugLog.h"

#include <imgui.h>

namespace Display
{

#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240

uint32_t palette[256];                                            				// Current DVI palette (RGB565)
uint8_t  screenMemory[FRAME_WIDTH * FRAME_HEIGHT];                                           				// Graphics RAM (max 320x240)
int frameCount = 0;                                            				// Frame counter
FGraphicsView* pGraphicsView = nullptr;                                            				// Pointer to graphics view for rendering

void SetBBCMicroDefaultPalette(void);

void Init(void)
{
	if (pGraphicsView == nullptr)
	{
		pGraphicsView = new FGraphicsView(FRAME_WIDTH, FRAME_HEIGHT);
	}
	ClearScreen(0); // Clear the screen to black
	SetBBCMicroDefaultPalette(); // Set the default BBC Micro palette
	frameCount = 0; // Reset frame count
}

int GetFrameCount(void)
{
	return frameCount;
}

void WaitVBlank(void)
{

}

// Set a palette entry
// colour is 0-255, r,g,b are 0-255
// 32 bit ABGR
void SetPalette(uint8_t colour, uint8_t r, uint8_t g, uint8_t b)
{
	palette[colour] = IM_COL32(r, g, b, 255); // Set the palette entry in ABGR format
}

// Set the first 8 palette entries to the default BBC Micro palette
void SetBBCMicroDefaultPalette(void)
{
	// BBC Micro default 8 colours (RGB values 0-255)
	// 0: Black, 1: Red, 2: Green, 3: Yellow, 4: Blue, 5: Magenta, 6: Cyan, 7: White
	static const uint8_t bbc_palette[8][3] = {
		{0, 0, 0},        // Black
		{255, 0, 0},      // Red
		{0, 255, 0},      // Green
		{255, 255, 0},    // Yellow
		{0, 0, 255},      // Blue
		{255, 0, 255},    // Magenta
		{0, 255, 255},    // Cyan
		{255, 255, 255}   // White
	};
	for (uint8_t i = 0; i < 8; ++i) 
	{
		SetPalette(i, bbc_palette[i][0], bbc_palette[i][1], bbc_palette[i][2]);
	}
}

// drawing functions
void ClearScreen(uint8_t colour)
{
	memset(screenMemory, colour, sizeof(screenMemory));                     		// Clear screen memory.
}

void ClearScreenFromYpos(int y, uint8_t col)
{
	if (y < 0 || y >= FRAME_HEIGHT)
		return; // Out of bounds

	memset(screenMemory + y * FRAME_WIDTH, col, (FRAME_HEIGHT - y) * FRAME_WIDTH); // Clear from y to end of screen
}

void DrawPixel(uint16_t x, uint16_t y, uint8_t colour)
{
	x += 32; // offset for Tube Elite display
	if (x < FRAME_WIDTH && y < FRAME_HEIGHT)
	{
		screenMemory[y * FRAME_WIDTH + x] = colour;                     		// Set pixel in screen memory.
	}
}

void DrawPixelEOR(uint16_t x, uint16_t y, uint8_t colour)
{
	x += 32; // offset for Tube Elite display
	if (x < FRAME_WIDTH && y < FRAME_HEIGHT)
	{
		screenMemory[y * FRAME_WIDTH + x] ^= colour;                     		// EOR pixel in screen memory.
	}
}

void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colour)
{
	// Simple Bresenham's line algorithm implementation
	const int dx = abs(x2 - x1);
	const int dy = abs(y2 - y1);
	const int sx = (x1 < x2) ? 1 : -1;
	const int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	while (true)
	{
		DrawPixel(x1, y1, colour);
		if (x1 == x2 && y1 == y2)
			break;
		const int err2 = err * 2;

		if (err2 > -dy)
		{
			err -= dy;
			x1 += sx;
		}

		if (err2 < dx)
		{
			err += dx;
			y1 += sy;
		}
	}
}

void DrawLineEOR(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colour)
{
	// Simple Bresenham's line algorithm implementation
	const int dx = abs(x2 - x1);
	const int dy = abs(y2 - y1);
	const int sx = (x1 < x2) ? 1 : -1;
	const int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	while (true)
	{
		if (x1 == x2 && y1 == y2)
			break;
		DrawPixelEOR(x1, y1, colour);

		const int err2 = err * 2;

		if (err2 > -dy)
		{
			err -= dy;
			x1 += sx;
		}

		if (err2 < dx)
		{
			err += dx;
			y1 += sy;
		}
	}
}

void DrawHLineEOR(uint16_t x1, uint16_t x2, uint16_t y, uint8_t colour)
{
	if (y < 0 || y >= FRAME_HEIGHT)
		return; // Out of bounds
	// Ensure x1 is less than x2
	if (x1 > x2)
		std::swap(x1, x2);
	// Clamp x1 and x2 to screen width
	if (x1 < 0) x1 = 0;
	if (x2 >= FRAME_WIDTH) x2 = FRAME_WIDTH - 1;
	for (uint16_t x = x1; x <= x2; ++x)
	{
		DrawPixelEOR(x, y, colour); // Draw each pixel in the horizontal line
	}
}

// Draw a single 8x8 character at (x, y) using BBC font, with top-left at (x, y)
void DrawChar8x8(uint16_t x, uint16_t y, uint8_t ch, uint8_t colour)
{
	if (ch < 32 || ch > 127)
		ch = 32; // fallback to space

	//const uint8_t* glyph = bbc_font_8x8[ch - 32];
	const uint8_t* glyph = &BBCFont[(ch - 32) * 8];
	for (int row = 0; row < 8; ++row)
	{
		const uint8_t bits = glyph[row];
		for (int col = 0; col < 8; ++col)
		{
			if (bits & (0x80 >> col))
			{
				DrawPixelEOR(x + col, y + row, colour);
			}
		}
	}
}
// Windows specific

void RenderFrame()
{
	if (pGraphicsView == nullptr)
	{
		LOGERROR("Graphics view not initialized!");
		return;
	}

	// expand screenMemory to 32-bit pixels
	uint32_t* pViewPixels = pGraphicsView->GetPixelBuffer();
	for(int i=0;i<FRAME_WIDTH * FRAME_HEIGHT;i++)
	{
		const uint8_t colour = screenMemory[i];
		const uint32_t col = palette[colour];
		pViewPixels[i] = col; // convert to 32-bit pixel
	}
	
	pGraphicsView->UpdateTexture();
	pGraphicsView->Draw();
}

} // namespace Display