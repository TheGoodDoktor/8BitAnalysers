#include "PCEGraphicsView.h"

#include <geargrafx_core.h>
#include "../PCEEmu.h"

// Monochrome sprites. Pass in as flag to Draw4bppSpriteImage()? Probably useless though.
bool gPCEGraphicsViewMonoSprites = false;

// height and width are in 16x16 sprite character blocks.
// paletteIndex is sprite palette index 0-15
void FPCEGraphicsView::Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int width, int height, int paletteIndex)
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();

	HuC6260* huc6260 = pCore->GetHuC6260();
	u16* colorTable = huc6260->GetColorTable();
	constexpr int paletteBaseIndex = 0x100;
	const u16* pPalette = &colorTable[paletteBaseIndex + (paletteIndex * 16)];

	constexpr int blockWidth = 16;
	constexpr int blockHeight = 16;

	int curXPos = xp;
	int curYPos = yp;

	const int viewWidth = GetWidth();
	uint32_t* pPixBuf = GetPixelBuffer();

	const uint16_t* pPlane0 = (uint16_t*)pSrc;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			uint32_t* pCurPixBuf = pPixBuf + (curXPos + (curYPos * viewWidth));
			const uint16_t* pPlane1 = pPlane0 + 16;
			const uint16_t* pPlane2 = pPlane1 + 16;
			const uint16_t* pPlane3 = pPlane2 + 16;

			// Draw 16x16 pixel square
			for (int y = 0; y < blockHeight; y++)
			{
				// Draw 16 pixel horiz line
				for (int x = 0; x < blockWidth; x++)
				{
					const int bit = (blockWidth - 1) - x;  
					// Get the 4 bit pixel colour index (0-15)
					const int colour = ((*pPlane3 >> bit) & 1) << 3 | ((*pPlane2 >> bit) & 1) << 2 | ((*pPlane1 >> bit) & 1) << 1 | ((*pPlane0 >> bit) & 1) & 0x1ff;

					if (colour != 0) // 0 is transparent
					{
						if (gPCEGraphicsViewMonoSprites)
						{
							*pCurPixBuf = 0xffffffff;
						}
						else
						{
							// Convert from 333 colour to u32 RGB
							constexpr int paletteBaseIndex = 0x100;
							const int colour333 = pPalette[colour];
							const uint8_t g = ((colour333 >> 6) & 0x07) * 255 / 7;
							const uint8_t r = ((colour333 >> 3) & 0x07) * 255 / 7;
							const uint8_t b = (colour333 & 0x07) * 255 / 7;
							*pCurPixBuf = (0xff << 24) | (b << 16) | (g << 8) | r;
						}
					}
					else
						*pCurPixBuf = 0xff000000;

					pCurPixBuf++;
				}

				pPlane0++;
				pPlane1++;
				pPlane2++;
				pPlane3++;

				pCurPixBuf += viewWidth - blockWidth;
			}
			curXPos += blockWidth;
			pPlane0 = pPlane3;
		}
		curYPos += blockHeight;
		curXPos = xp;
	}
}