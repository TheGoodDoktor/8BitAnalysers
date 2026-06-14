#include "PCEGraphicsView.h"

#include <geargrafx_core.h>
#include "../PCEEmu.h"

// Converts a 16-entry PCE palette (333 colour) to RGB, with colour 0 forced transparent.
static void ConvertPCEPaletteToCols(const u16* pPalette, uint32_t* cols)
{
	cols[0] = 0xff000000; // colour 0 is transparent

	for (int i = 1; i < 16; i++)
	{
		const int colour333 = pPalette[i];
		const uint8_t g = ((colour333 >> 6) & 0x07) * 255 / 7;
		const uint8_t r = ((colour333 >> 3) & 0x07) * 255 / 7;
		const uint8_t b = (colour333 & 0x07) * 255 / 7;
		cols[i] = (0xff << 24) | (b << 16) | (g << 8) | r;
	}
}

// height and width are in 16x16 sprite character blocks.
// paletteIndex is sprite palette index 0-15
void FPCEGraphicsView::Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int width, int height, int paletteIndex)
{
	HuC6260* huc6260 = pPCEEmu->GetCore()->GetHuC6260();
	u16* colorTable = huc6260->GetColorTable();
	constexpr int paletteBaseIndex = 0x100;
	const u16* pPalette = &colorTable[paletteBaseIndex + ((paletteIndex & 0xf) * 16)];

	uint32_t cols[16];
	ConvertPCEPaletteToCols(pPalette, cols);

	Draw4bpp16x16PlanarSpriteImage(pSrc, xp, yp, width, height, cols);
}

// height and width are in 8x8 BG tile blocks.
// paletteIndex is BG palette index 0-15
void FPCEGraphicsView::Draw4bppBGTileImage(const uint8_t* pSrc, int xp, int yp, int width, int height, int paletteIndex)
{
	HuC6260* huc6260 = pPCEEmu->GetCore()->GetHuC6260();
	u16* colorTable = huc6260->GetColorTable();
	constexpr int paletteBaseIndex = 0x000;
	const u16* pPalette = &colorTable[paletteBaseIndex + ((paletteIndex & 0xf) * 16)];

	uint32_t cols[16];
	ConvertPCEPaletteToCols(pPalette, cols);

	Draw4bpp8x8PlanarBGTileImage(pSrc, xp, yp, width, height, cols);
}