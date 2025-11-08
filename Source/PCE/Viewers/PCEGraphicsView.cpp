#include "PCEGraphicsView.h"

#include <geargrafx_core.h>
#include "../PCEEmu.h"

void FPCEGraphicsView::Draw4bppSpriteImage(const uint8_t* pSrc, int xp, int yp, int widthPixels, int heightPixels, int paletteIndex)
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();

	HuC6260* huc6260 = pCore->GetHuC6260();
	HuC6270* huc6270 = pCore->GetHuC6270_1();
	u16* sat = huc6270->GetSAT();
	u16* color_table = huc6260->GetColorTable();

	const uint32_t segCols[5] = { 0xffff0000, 0xff00ff00, 0xff0000ff, 0xffff00ff, 0xffffff00 };

	// Split up into segments.
	const int segWidth = 16;
	const int segHeight = 16;
	const int segCountV = widthPixels / segWidth;
	const int segCountH = heightPixels / segHeight;

	int curXPos = xp;
	int curYPos = yp;

	const int width = GetWidth();
	uint32_t* pPixBuf = GetPixelBuffer();

	const uint16_t* pPlane0 = (uint16_t*)pSrc;

	for (int segY = 0; segY < segCountV; segY++)
	{
		for (int segX = 0; segX < segCountH; segX++)
		{
			uint32_t* pCurPixBuf = pPixBuf + (curXPos + (curYPos * width));
			const uint16_t* pPlane1 = pPlane0 + 16;
			const uint16_t* pPlane2 = pPlane1 + 16;
			const uint16_t* pPlane3 = pPlane2 + 16;

			// Draw 16x16 pixel square
			for (int y = 0; y < segHeight; y++)
			{
				// Draw 16 pixel horiz line
				for (int x = 0; x < segWidth; x++)
				{
					const int bit = 15 - x;  
					int color = ((*pPlane3 >> bit) & 1) << 3 | ((*pPlane2 >> bit) & 1) << 2 | ((*pPlane1 >> bit) & 1) << 1 | ((*pPlane0 >> bit) & 1);

					if (color != 0) // 0 is transparent
						*pCurPixBuf = 0xffffffff;
					else
						*pCurPixBuf = 0xff000000;

					pCurPixBuf++;
				}

				pPlane0++;
				pPlane1++;
				pPlane2++;
				pPlane3++;

				pCurPixBuf += width - segWidth;
			}
			curXPos += segWidth;
			pPlane0 = pPlane3;
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