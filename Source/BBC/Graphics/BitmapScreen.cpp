#include "BitmapScreen.h"
#include <cstdint>
#include "../BBCEmu/BBCEmu.h"
#include "../BBCEmulator.h"
#include "Util/GraphicsView.h"

uint16_t WrapAddress(FBBCEmulator* pBBCEmu, uint16_t addr) 
{
	const uint16_t offsets[] = { 0x4000,0x6000,0x3000,0x5800 };
	if (addr < 0x8000)
		return addr;

	addr += offsets[(pBBCEmu->GetBBC().ic32 & IC32_LATCH_SCREENADDR_MASK) >> 4];
	return addr & 0x7fff;
}

void UpdateGraphicsScreen1bpp(FBBCEmulator* pBBCEmu, FGraphicsView* pScreenView)
{
	const uint32_t cols[] = { pBBCEmu->GetColour(0),pBBCEmu->GetColour(8) };	// this is because of the way the ULA works
	const bbc_t& bbc = pBBCEmu->GetBBC();
	const mc6845_t& crtc = bbc.crtc;

	const int widthChars = crtc.h_displayed;
	const int heightChars = crtc.v_displayed;
	const int characterHeight = bbc.video_ula.teletext ? 20 : crtc.max_scanline_addr + 1;
	const int characterWidth = bbc.video_ula.teletext ? 16 : 8;

	const int screenWidth = widthChars * characterWidth;
	const int screenHeight = heightChars * characterHeight;
	const uint16_t displayAddress = (crtc.start_addr_hi << 8) | crtc.start_addr_lo;

	uint16_t currentScreenAddress = displayAddress * 8;
	const uint16_t cursorAddress = WrapAddress(pBBCEmu,((crtc.cursor_hi << 8) | crtc.cursor_lo) * 8);
	const int charByteSize = 8;
	const int charLineByteSize = charByteSize * widthChars;

	for (int y = 0; y < heightChars; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			const uint16_t charAddress = WrapAddress(pBBCEmu, currentScreenAddress + ((y * charLineByteSize) + (x * charByteSize)));
			for (int line = 0; line < characterHeight; line++)
			{
				const uint8_t charLine = pBBCEmu->ReadByte(WrapAddress(pBBCEmu, charAddress + line));
				pScreenView->DrawCharLine(charLine, x * characterWidth, (y * characterHeight) + line, cols[1], cols[0]);
			}
			// Draw cursor
			if (charAddress == cursorAddress)
				pScreenView->DrawCharLine(0xff, x * characterWidth, (y * characterHeight) + characterHeight - 1, cols[1], cols[0]);
		}
	}
}

void UpdateGraphicsScreen2bpp(FBBCEmulator* pBBCEmu, FGraphicsView* pScreenView)
{
	// 0 2 8 10
	const uint32_t cols[] = { pBBCEmu->GetColour(0),pBBCEmu->GetColour(2),pBBCEmu->GetColour(8),pBBCEmu->GetColour(10) };	// TODO: get colours from palette
	const bbc_t& bbc = pBBCEmu->GetBBC();
	const mc6845_t& crtc = bbc.crtc;

	const int widthChars = crtc.h_displayed;
	const int heightChars = crtc.v_displayed;
	const int characterHeight = bbc.video_ula.teletext ? 20 : crtc.max_scanline_addr + 1;
	const int characterWidth = bbc.video_ula.teletext ? 16 : 8;

	const int screenWidth = widthChars * characterWidth;
	const int screenHeight = heightChars * characterHeight;
	const uint16_t displayAddress = (crtc.start_addr_hi << 8) | crtc.start_addr_lo; 

	uint16_t currentScreenAddress = displayAddress * 8;
	const int charByteSize = 8;
	const int charLineByteSize = charByteSize * widthChars;
	const uint16_t cursorAddress = WrapAddress(pBBCEmu, ((crtc.cursor_hi << 8) | crtc.cursor_lo) * 8);

	for (int y = 0; y < heightChars; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			int xp = x * characterWidth;
			int yp = y * characterHeight;
			uint32_t* pBase = pScreenView->GetPixelBuffer() + (xp + (yp * pScreenView->GetWidth()));

			const uint16_t charAddress = WrapAddress(pBBCEmu, currentScreenAddress + ((y * charLineByteSize) + (x * charByteSize)));
			const uint8_t* pChar = pBBCEmu->GetMemPtr(charAddress);

			for (int l = 0; l < characterHeight; l++)
			{
				const uint8_t charLine = pBBCEmu->ReadByte(WrapAddress(pBBCEmu, charAddress + l));

				for (int c = 0; c < characterWidth / 2; c++)
				{
					const int shift = ((characterWidth / 2) - 1) - c;
					const uint8_t pixelVal = ((charLine >> shift) & 1) | ((charLine & (1 << (4 + shift))) >> (3 + shift));

					*pBase++ = cols[pixelVal];
					*pBase++ = cols[pixelVal];
				}
				pChar++;
				pBase += pScreenView->GetWidth() - characterWidth;
			}

			// Draw cursor
			if (charAddress == cursorAddress)
				pScreenView->DrawCharLine(0xff, x * characterWidth, (y * characterHeight) + characterHeight - 1, cols[3], cols[0]);

		}
	}
}

void UpdateGraphicsScreen4bpp(FBBCEmulator* pBBCEmu, FGraphicsView* pScreenView)
{
	const bbc_t& bbc = pBBCEmu->GetBBC();
	const mc6845_t& crtc = bbc.crtc;

	const int widthChars = crtc.h_displayed;
	const int heightChars = crtc.v_displayed;
	const int characterHeight = bbc.video_ula.teletext ? 20 : crtc.max_scanline_addr + 1;
	const int characterWidth = bbc.video_ula.teletext ? 16 : 8;

	const int screenWidth = widthChars * characterWidth;
	const int screenHeight = heightChars * characterHeight;
	const uint16_t displayAddress = (crtc.start_addr_hi << 8) | crtc.start_addr_lo;

	uint16_t currentScreenAddress = displayAddress * 8;
	const int charByteSize = 8;
	const int charLineByteSize = charByteSize * widthChars;
	const uint16_t cursorAddress = WrapAddress(pBBCEmu, ((crtc.cursor_hi << 8) | crtc.cursor_lo) * 8);

	for (int y = 0; y < heightChars; y++)
	{
		for (int x = 0; x < widthChars; x++)
		{
			int xp = x * characterWidth;
			int yp = y * characterHeight;
			uint32_t* pBase = pScreenView->GetPixelBuffer() + (xp + (yp * pScreenView->GetWidth()));

			const uint16_t charAddress = WrapAddress(pBBCEmu, currentScreenAddress + ((y * charLineByteSize) + (x * charByteSize)));

			for (int l = 0; l < characterHeight; l++)
			{
				const uint8_t charLine = pBBCEmu->ReadByte(WrapAddress(pBBCEmu, charAddress + l));
				const uint8_t col0 = ((charLine & 2) >> 1) | ((charLine & 8) >> 2) | ((charLine & 0x20) >> 3) | ((charLine & 0x80) >> 4);
				const uint8_t col1 = ((charLine & 1) >> 0) | ((charLine & 4) >> 1) | ((charLine & 0x10) >> 2) | ((charLine & 0x40) >> 3);

				for (int c = 0; c < characterWidth / 4; c++)
				{
					uint32_t pixelCol = pBBCEmu->GetColour(c == 0 ? col0 : col1);
					*pBase++ = pixelCol;
					*pBase++ = pixelCol;
					*pBase++ = pixelCol;
					*pBase++ = pixelCol;
				}
				pBase += pScreenView->GetWidth() - characterWidth;
			}

			// Draw cursor
			if (charAddress == cursorAddress)
				pScreenView->DrawCharLine(0xff, x * characterWidth, (y * characterHeight) + characterHeight - 1, pBBCEmu->GetColour(7), pBBCEmu->GetColour(0));
		}
	}
}

void UpdateScreenPixelImage(FBBCEmulator* pBBCEmu, FGraphicsView* pScreenView)
{
	const FCodeAnalysisState& state = pBBCEmu->GetCodeAnalysis();

	const bbc_t& bbc = pBBCEmu->GetBBC();
	const mc6845_t& crtc = bbc.crtc;

	const int bpp = crtc.h_displayed / bbc.video_ula.num_chars_per_line;

	switch (bpp)
	{
	case 1:
		UpdateGraphicsScreen1bpp(pBBCEmu, pScreenView);
		break;
	case 2:
		UpdateGraphicsScreen2bpp(pBBCEmu, pScreenView);
		break;
	case 4:
		UpdateGraphicsScreen4bpp(pBBCEmu, pScreenView);
		break;
	}

}

