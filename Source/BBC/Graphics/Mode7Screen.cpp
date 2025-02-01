#include "Mode7Screen.h"
#include "../BBCEmulator.h"
#include "Util/GraphicsView.h"

#include "ttxfont.h"


void DrawMode7ScreenToGraphicsView(FBBCEmulator* pBBCEmu, FGraphicsView* pScreenView)
{
	const bbc_t& bbc = pBBCEmu->GetBBC();
	int widthChars = 40;
	int heightChars = 25;

	int characterHeight = 20;
	int characterWidth = 16;

	int screenWidth = widthChars * characterWidth;
	int screenHeight = heightChars * characterHeight;
	const mc6845_t& crtc = bbc.crtc;
	const uint16_t displayAddress = (crtc.start_addr_hi << 8) | crtc.start_addr_lo;

	uint16_t currentScreenAddress = displayAddress;//(bbc.crtc.ma & 0x800) << 3 | 0x3C00 | (bbc.crtc.ma & 0x3FF);

	for (int y = 0; y < heightChars; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint16_t ttxScreenAddress = (currentScreenAddress & 0x800) << 3 | 0x3C00 | (currentScreenAddress & 0x3FF);
			const int charCode = pBBCEmu->ReadByte(ttxScreenAddress);
			currentScreenAddress++;

			if (charCode < 0x20)
			{
				// TODO: process control character
			}
			else if (charCode < 0x20 + 96)
			{
				const uint16_t* pChar = mode7font[charCode - 0x20];

				for (int l = 0; l < 20; l++)
				{
					uint32_t* pCurPixBufAddr = pScreenView->GetPixelBuffer() + (screenWidth * (y * 20 + l)) + (x * 16);
					uint16_t characterLine = pChar[l];
					for (int c = 0; c < 16; c++)
					{
						const uint32_t pixelColour = characterLine & 0x8000 ? 0xffffffff : 0xff000000;
						*pCurPixBufAddr = pixelColour;
						pCurPixBufAddr++;
						characterLine <<= 1;
					}
				}
			}

		}
	}
}