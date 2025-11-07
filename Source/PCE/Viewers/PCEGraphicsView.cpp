#include "PCEGraphicsView.h"

#include <geargrafx_core.h>
#include "../PCEEmu.h"

int gSegWidth = 16;
int gSegHeight = 16;

void FPCEGraphicsView::Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();

	HuC6260* huc6260 = pCore->GetHuC6260();
	HuC6270* huc6270 = pCore->GetHuC6270_1();
	u16* sat = huc6270->GetSAT();
	u16* color_table = huc6260->GetColorTable();

	// 16x16
	// $80 = 128
	// 1 byte = 2 pixels
	// num bytes = widthPixels / 2
	const uint8_t* pPlane1 = pSrc;
	const uint8_t* pPlane2 = pPlane1 + 8;
	const uint8_t* pPlane3 = pPlane2 + 8;
	const uint8_t* pPlane4 = pPlane3 + 8;

	const uint32_t segCols[5] = { 0xffff0000, 0xff00ff00, 0xff0000ff, 0xffff00ff, 0xffffff00 };
	int curSeg = 0;

	// Split up into segments.
	const int segWidth = gSegWidth;
	const int segHeight = gSegHeight;
	const int vertSegCount = widthPixels / segWidth;
	const int horizSegCount = heightPixels / segHeight;

	int curXPos = xp;
	int curYPos = yp;

	int width = GetWidth();
	uint32_t* pPixBuf = GetPixelBuffer();

	for (int segY = 0; segY < vertSegCount; segY++)
	{
		for (int segX = 0; segX < horizSegCount; segX++)
		{
			uint32_t* pCur = pPixBuf + (curXPos + (curYPos * width));

			for (int y = 0; y < segHeight; y++)
			{
				for (int x = 0; x < segWidth; x++)
				{
					//*(pBase + x + (x * 8)) = 0xFFFF0000;
					//*pCur = 0xffff0000;
					//assert(curSeg < 4);
					*pCur = segCols[curSeg % 5];
					//pCol++;
					pCur++;
					//pPlane1++;
					//pPlane2++;
					//pPlane3++;
					//pPlane4++;
				}
				// calculate this before?
				pCur += width - segWidth;
			}
			curXPos += segWidth;
			curSeg++;
		}
		curYPos += segHeight;
		curXPos = xp;
	}

#if 0
	for (int y = 0; y < height; y++)
	{
		const int tile_y = y >> 4;
		const int tile_line_offset = tile_y * 2 * 64;
		const int offset_y = y & 0xF;
		const u16 line_start = sprite_address + tile_line_offset + offset_y;

		for (int x = 0; x < width; x++)
		{
			const int tile_x = x >> 4;
			const int tile_x_offset = tile_x * 64;
			const int line = line_start + tile_x_offset + mode1_offset;

			const u16 plane1 = vram[line + 0];
			const u16 plane2 = vram[line + 16];
			const u16 plane3 = vram[line + 32];
			const u16 plane4 = vram[line + 48];

			const int pixel_x = 15 - (x & 0xF);
			u16 pixel = ((plane1 >> pixel_x) & 0x01) | (((plane2 >> pixel_x) & 0x01) << 1) | (((plane3 >> pixel_x) & 0x01) << 2) | (((plane4 >> pixel_x) & 0x01) << 3);
			pixel |= (palette << 4);
			pixel |= 0x100;

			const int color = color_table[pixel & 0x1FF];
			u8 green = ((color >> 6) & 0x07) * 255 / 7;
			u8 red = ((color >> 3) & 0x07) * 255 / 7;
			u8 blue = (color & 0x07) * 255 / 7;

			if (!(pixel & 0x0F))
			{
				//if (BackgroundColour == 0)
				{
					// Grey
					red = 128;
					green = 128;
					blue = 128;
				}
#if 0
				else if (BackgroundColour == 1)
				{
					// Black
					red = 0;
					green = 0;
					blue = 0;
				}
				else
				{
					// Magenta
					red = 255;
					green = 0;
					blue = 255;
				}
#endif
			}

			int pixel_index = ((y * width) + x) << 2;
			SpriteBuffers[i][pixel_index + 0] = red;
			SpriteBuffers[i][pixel_index + 1] = green;
			SpriteBuffers[i][pixel_index + 2] = blue;
			SpriteBuffers[i][pixel_index + 3] = 255;
		}
	}
#endif
}

#if 0
void FPCEGraphicsView::DrawMode1Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	if (const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex))
	{
		FGraphicsView::Draw2BppImageAt(pSrc, xp, yp, widthPixels, heightPixels, pPalette);
	}
}

void FPCEGraphicsView::DrawMode0Image(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	if (const FPaletteEntry* pPaletteEntry = GetPaletteEntry(paletteIndex))
	{
		if (pPaletteEntry->NoColours < 16)
			return;

		const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex);
		if (pPalette)
		{
			FGraphicsView::Draw4BppWideImageAt(pSrc, xp, yp, widthPixels, heightPixels, pPalette);
		}
	}
}
#endif