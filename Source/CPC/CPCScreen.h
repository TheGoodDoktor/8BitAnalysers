#pragma once

#include "chips/chips_common.h"
#include "chips/am40010.h"

#include <Util/GraphicsView.h>

class FCpcEmu;

// A class of helper functions related to the CPC screen.
class FCPCScreen
{
public:
	void Init(FCpcEmu* pEmu);
	void Tick();
	void Reset();

	int GetTopScanline() const { return ScreenTopScanline; }
	int GetLeftEdgeScanline() const { return ScreenLeftEdgeOffset; }
	int GetScreenModeForScanline(int scanline) const;
	int GetScreenModeForYPos(int yPos) const;
	const FPalette& GetPaletteForScanline(int scanline) const;
	const FPalette& GetPaletteForYPos(int yPos) const;

	uint16_t GetScreenAddrStart() const;
	uint16_t GetScreenAddrEnd() const;
	uint16_t GetScreenMemSize() const;

	bool IsScrolled() const;

	// Get a screen memory address for a screen position.
	bool GetScreenMemoryAddress(int x, int y, uint16_t& addr) const;

	// Get the position on the screen, given a screen memory address.
	bool GetScreenAddressCoords(uint16_t addr, int& x, int& y) const;

private:
	// Note: the screen mode (on real HW anyway) can, in theory, be changed mid-scanline. 
	// We don't currently support this. Don't know if CHIPS supports this or if any games do this.

	// should this be AM40010_FRAMEBUFFER_HEIGHT? this is 272. 
	// I thought the cpc had 312 scanlines?
	int ScreenModePerScanline[AM40010_DISPLAY_HEIGHT] = { -1 };
	FPalette PalettePerScanline[AM40010_DISPLAY_HEIGHT];
	int LastScanline = -1;
	bool bInVblank = false;
	bool bDrawingPixels = false;
	int ScreenTopScanline = 0;
	int ScreenLeftEdgeOffset = 0;

	FCpcEmu* pCpcEmu;
};

int GetHWColourIndexForPixel(uint8_t val, int pixelIndex, int scrMode);
uint8_t GetBitsPerPixel(int screenMode);
