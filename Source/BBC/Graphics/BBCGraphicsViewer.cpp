#include "BBCGraphicsViewer.h"
#include "../BBCEmulator.h"
#include "../BBCChipsImpl.h"
#include "BBCGraphicsView.h"

#include <imgui.h>
#include <CodeAnalyser/UI/UIColours.h>

// TODO: move this to the emulator class


FBBCGraphicsViewer::FBBCGraphicsViewer(FBBCEmulator* pEmu)
	: FGraphicsViewer(pEmu)
{
	pBBCEmu = (FBBCEmulator*)pEmulator;
}

bool FBBCGraphicsViewer::Init()
{
	FGraphicsViewer::Init();

	return true;
}

void FBBCGraphicsViewer::Shutdown()
{
	FGraphicsViewer::Shutdown();
}

void FBBCGraphicsViewer::DrawUI()
{
	FGraphicsViewer::DrawUI();
}

int BBCKeyFromImGuiKey(ImGuiKey key);

void FBBCGraphicsViewer::DrawScreenViewer()
{
	const bbc_t& bbc = pBBCEmu->GetBBC();
	const mc6845_t& crtc = bbc.crtc;

	//uint16_t crtcStartAddress = (crtc.start_addr_hi << 8) | crtc.start_addr_lo;

	DisplayAddress = (crtc.start_addr_hi << 8) | crtc.start_addr_lo;
	WidthChars = crtc.h_displayed;
	HeightChars = crtc.v_displayed;
	CharacterHeight = crtc.max_scanline_addr + 1;

	ImGui::Text("Screen width chars: %d x %d", WidthChars, HeightChars);
	ImGui::Text("Character height: %d", CharacterHeight);
	ImGui::Text("Display address: 0x%04x", DisplayAddress);
	ImGui::Text("Teletext: %s", bbc.video_ula.teletext ? "Yes" : "No");
	ImGui::Text("Caps Lock: %s", (bbc.ic32 & IC32_LATCH_CAPS_LOCK_LED) == 0 ? "On" : "Off");
	ImGui::Text("Shift Lock: %s", (bbc.ic32 & IC32_LATCH_SHIFT_LOCK_LED) == 0 ? "On" : "Off");


	//UpdateScreenPixelImage();
	if (bbc.video_ula.teletext)
		UpdateScreenTeletextImage();
	else
		UpdateScreenPixelImage();
	pScreenView->Draw(320,256);

	// big hack (tm)
	// Check keys - not event driven, hopefully perf isn't too bad
	for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_COUNT; key++)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)key, false))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0)// && bWindowFocused)
				bbc_key_down(&pBBCEmu->GetBBC(), bbcKey);
		}
		else if (ImGui::IsKeyReleased((ImGuiKey)key))
		{
			const int bbcKey = BBCKeyFromImGuiKey((ImGuiKey)key);
			if (bbcKey != 0)
				bbc_key_up(&pBBCEmu->GetBBC(), bbcKey);
		}
	}
}

int GetHWColourIndexForPixel(uint8_t val, int pixelIndex, int scrMode);

uint32_t FBBCGraphicsViewer::GetRGBValueForPixel(int colourIndex, uint32_t heatMapCol) const
{
	const ImColor colour = pPaletteColours ? pPaletteColours[colourIndex] : colourIndex == 0 ? 0xff000000 : 0xffffffff;

	if (!bShowReadsWrites)
		return colour;

	// Grayscale value is R * 0.299 + G * 0.587 + B * 0.114
	const float grayScaleValue = colour.Value.x * 0.299f + colour.Value.y * 0.587f + colour.Value.z * 0.114f;
	const ImColor grayScaleColour = ImColor(grayScaleValue, grayScaleValue, grayScaleValue);
	ImU32 finalColour = grayScaleColour;
	if (heatMapCol != 0xffffffff)
	{
		finalColour |= heatMapCol;
	}
	return finalColour;
}

uint16_t FBBCGraphicsViewer::WrapAddress(uint16_t addr) const
{
	const uint16_t offsets[] = {0x4000,0x6000,0x3000,0x5800};
	if (addr < 0x8000)
		return addr;

	addr += offsets[(pBBCEmu->GetBBC().ic32 & IC32_LATCH_SCREENADDR_MASK)>>4];
	return addr & 0x7fff;
}

void FBBCGraphicsViewer::UpdateGraphicsScreen1bpp()
{
	const uint32_t cols[] = { pBBCEmu->GetColour(0),pBBCEmu->GetColour(8) };	// this is because of the way the ULA works
	uint16_t currentScreenAddress = DisplayAddress * 8;
	mc6845_t& crtc = pBBCEmu->GetBBC().crtc;
	const uint16_t cursorAddress = WrapAddress(((crtc.cursor_hi << 8) | crtc.cursor_lo) * 8);
	const int charByteSize = 8;
	const int charLineByteSize = charByteSize * WidthChars;

	for (int y = 0; y < HeightChars; y++)
	{
		for (int x = 0; x < WidthChars; x++)
		{
			const uint16_t charAddress = WrapAddress(currentScreenAddress + ((y * charLineByteSize) + (x * charByteSize)));
			for (int line = 0; line < CharacterHeight; line++)
			{
				const uint8_t charLine = pBBCEmu->ReadByte(WrapAddress(charAddress + line));
				pScreenView->DrawCharLine(charLine, x * CharacterWidth, (y * CharacterHeight) + line, cols[1], cols[0]);
			}
			// Draw cursor
			if (charAddress == cursorAddress)
				pScreenView->DrawCharLine(0xff, x * CharacterWidth, (y * CharacterHeight) + CharacterHeight-1, cols[1], cols[0]);
		}
	}
}

void FBBCGraphicsViewer::UpdateGraphicsScreen2bpp()
{
	// 0 2 8 10
	const uint32_t cols[] = { pBBCEmu->GetColour(0),pBBCEmu->GetColour(2),pBBCEmu->GetColour(8),pBBCEmu->GetColour(10) };	// TODO: get colours from palette
	uint16_t currentScreenAddress = DisplayAddress * 8;
	const int charByteSize = 8;
	const int charLineByteSize = charByteSize * WidthChars;
	mc6845_t& crtc = pBBCEmu->GetBBC().crtc;
	const uint16_t cursorAddress = WrapAddress(((crtc.cursor_hi << 8) | crtc.cursor_lo) * 8);

	for (int y = 0; y < HeightChars; y++)
	{
		for (int x = 0; x < WidthChars; x++)
		{
			int xp = x * CharacterWidth;
			int yp = y * CharacterHeight;
			uint32_t* pBase = pScreenView->GetPixelBuffer() + (xp + (yp * pScreenView->GetWidth()));

			const uint16_t charAddress = WrapAddress(currentScreenAddress + ((y * charLineByteSize) + (x * charByteSize)));
			const uint8_t* pChar = pBBCEmu->GetMemPtr(charAddress);

			for (int l = 0; l < CharacterHeight; l++)
			{
				const uint8_t charLine = pBBCEmu->ReadByte(WrapAddress(charAddress + l));

				for (int c = 0; c < CharacterWidth / 2; c++)
				{
					const int shift = ((CharacterWidth / 2) -1) - c;
					const uint8_t pixelVal = ((charLine >> shift) & 1) | ((charLine & (1 << (4+ shift))) >> (3+ shift));

					*pBase++ = cols[pixelVal];
					*pBase++ = cols[pixelVal];
				}
				pChar ++;
				pBase += pScreenView->GetWidth() - CharacterWidth;
			}

			// Draw cursor
			if (charAddress == cursorAddress)
				pScreenView->DrawCharLine(0xff, x * CharacterWidth, (y * CharacterHeight) + CharacterHeight - 1, cols[3], cols[0]);

		}
	}
}

void FBBCGraphicsViewer::UpdateGraphicsScreen4bpp()
{
	uint16_t currentScreenAddress = DisplayAddress * 8;
	const int charByteSize = 8;
	const int charLineByteSize = charByteSize * WidthChars;
	mc6845_t& crtc = pBBCEmu->GetBBC().crtc;
	const uint16_t cursorAddress = WrapAddress(((crtc.cursor_hi << 8) | crtc.cursor_lo) * 8);

	for (int y = 0; y < HeightChars; y++)
	{
		for (int x = 0; x < WidthChars; x++)
		{
			int xp = x * CharacterWidth;
			int yp = y * CharacterHeight;
			uint32_t* pBase = pScreenView->GetPixelBuffer() + (xp + (yp * pScreenView->GetWidth()));

			const uint16_t charAddress = WrapAddress(currentScreenAddress + ((y * charLineByteSize) + (x * charByteSize)));

			for (int l = 0; l < CharacterHeight; l++)
			{
				const uint8_t charLine = pBBCEmu->ReadByte(WrapAddress(charAddress + l));
				const uint8_t col0 = ((charLine & 2) >> 1) | ((charLine & 8) >> 2) | ((charLine & 0x20) >> 3) | ((charLine & 0x80) >> 4);
				const uint8_t col1 = ((charLine & 1) >> 0) | ((charLine & 4) >> 1) | ((charLine & 0x10) >> 2) | ((charLine & 0x40) >> 3);

				for (int c = 0; c < CharacterWidth / 4; c++)
				{
					uint32_t pixelCol = pBBCEmu->GetColour(c == 0 ? col0 : col1);
					*pBase++ = pixelCol;
					*pBase++ = pixelCol;
					*pBase++ = pixelCol;
					*pBase++ = pixelCol;
				}
				pBase += pScreenView->GetWidth() - CharacterWidth;
			}

			// Draw cursor
			if (charAddress == cursorAddress)
				pScreenView->DrawCharLine(0xff, x * CharacterWidth, (y * CharacterHeight) + CharacterHeight - 1, pBBCEmu->GetColour(7), pBBCEmu->GetColour(0));
		}
	}
}

void FBBCGraphicsViewer::UpdateScreenPixelImage(void)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();

	const bbc_t& bbc = pBBCEmu->GetBBC();
	const mc6845_t& crtc = bbc.crtc;
	
	const int lastScreenWidth = ScreenWidth;
	const int lastScreenHeight = ScreenHeight;

	CharacterWidth = 8;
	CharacterHeight = 8;

	ScreenWidth = WidthChars * CharacterWidth;
	ScreenHeight = HeightChars * CharacterHeight;

	if (ScreenWidth != lastScreenWidth || ScreenHeight != lastScreenHeight)
	{
		delete pScreenView;
		pScreenView = new FGraphicsView(ScreenWidth, ScreenHeight);
	}

	const int bpp = crtc.h_displayed / bbc.video_ula.num_chars_per_line;

	switch (bpp)
	{
	case 1:
		UpdateGraphicsScreen1bpp();
		break;
	case 2:
		UpdateGraphicsScreen2bpp();
		break;
	case 4:
		UpdateGraphicsScreen4bpp();
		break;
	}

}


// Given a byte containing multiple pixels, decode the colour index for a specified pixel.
// For screen mode 0 pixel index can be [0-1]. For mode 1 pixel index can be [0-3]
// This returns a colour index into the 32 hardware colours. It does not return an RGB value.
int GetHWColourIndexForPixel(uint8_t val, int pixelIndex, int scrMode)
{
	int colourIndex = -1;

	if (scrMode == 0)
	{
		if (pixelIndex == 0)
			colourIndex = (val & 0x80 ? 1 : 0) | (val & 0x8 ? 2 : 0) | (val & 0x20 ? 4 : 0) | (val & 0x2 ? 8 : 0);
		else if (pixelIndex == 1)
			colourIndex = (val & 0x40 ? 1 : 0) | (val & 0x4 ? 2 : 0) | (val & 0x10 ? 4 : 0) | (val & 0x1 ? 8 : 0);
	}
	else
	{
		switch (pixelIndex)
		{
		case 0:
			colourIndex = (val & 0x8 ? 2 : 0) | (val & 0x80 ? 1 : 0);
			break;
		case 1:
			colourIndex = (val & 0x4 ? 2 : 0) | (val & 0x40 ? 1 : 0);
			break;
		case 2:
			colourIndex = (val & 0x2 ? 2 : 0) | (val & 0x20 ? 1 : 0);
			break;
		case 3:
			colourIndex = (val & 0x1 ? 2 : 0) | (val & 0x10 ? 1 : 0);
			break;
		}
	}
	return colourIndex;
}

// Teletext

#include "ttxfont.h"

void	FBBCGraphicsViewer::UpdateScreenTeletextImage()
{
	const int lastScreenWidth = ScreenWidth;
	const int lastScreenHeight = ScreenHeight;

	WidthChars = 40;
	HeightChars = 25;

	CharacterHeight = 20;
	CharacterWidth = 16;

	ScreenWidth = WidthChars * CharacterWidth;
	ScreenHeight = HeightChars * CharacterHeight;

	if (ScreenWidth != lastScreenWidth || ScreenHeight != lastScreenHeight)
	{
		// temp. recreate the screen view
		// todo: need to create this up front. 
		delete pScreenView;
		pScreenView = new FGraphicsView(ScreenWidth, ScreenHeight);
	}

	const bbc_t& bbc = pBBCEmu->GetBBC();

	uint16_t currentScreenAddress = DisplayAddress;//(bbc.crtc.ma & 0x800) << 3 | 0x3C00 | (bbc.crtc.ma & 0x3FF);

	for (int y = 0; y < HeightChars; y++)
	{
		for (int x = 0; x < WidthChars; x++)
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
					uint32_t* pCurPixBufAddr = pScreenView->GetPixelBuffer() + (ScreenWidth * (y * 20 + l)) + (x * 16);
					uint16_t characterLine = pChar[l];
					for (int c = 0; c < 16; c++)
					{
						ImU32 pixelColour = characterLine & 0x8000 ? 0xffffffff : 0xff000000;
						*pCurPixBufAddr = pixelColour;
						pCurPixBufAddr++;
						characterLine <<= 1;
					}
				}
			}

		}
	}
}

struct FKeyVal
{
    int NoShift;
    int Shifted;
};

static std::map<ImGuiKey, FKeyVal> g_BBCKeysLUT = 
{
	{ImGuiKey_Space,		{BBC_KEYCODE_SPACE, BBC_KEYCODE_SPACE}},
	{ImGuiKey_Enter,		{BBC_KEYCODE_ENTER, BBC_KEYCODE_ENTER}},
	{ImGuiKey_Escape,		{BBC_KEYCODE_ESCAPE, BBC_KEYCODE_ESCAPE}},
	{ImGuiKey_LeftCtrl,		{BBC_KEYCODE_CTRL, BBC_KEYCODE_CTRL}},
	{ImGuiKey_RightCtrl,	{BBC_KEYCODE_CTRL, BBC_KEYCODE_CTRL}},
	{ImGuiKey_LeftShift,	{BBC_KEYCODE_SHIFT, BBC_KEYCODE_SHIFT}},
	{ImGuiKey_RightShift,	{BBC_KEYCODE_SHIFT, BBC_KEYCODE_SHIFT}},
	{ImGuiKey_Backspace,	{BBC_KEYCODE_BACKSPACE, BBC_KEYCODE_BACKSPACE}},
	{ImGuiKey_LeftArrow,	{BBC_KEYCODE_CURSOR_LEFT, BBC_KEYCODE_CURSOR_LEFT}},
	{ImGuiKey_RightArrow,	{BBC_KEYCODE_CURSOR_RIGHT, BBC_KEYCODE_CURSOR_RIGHT}},
	{ImGuiKey_UpArrow,		{BBC_KEYCODE_CURSOR_UP, BBC_KEYCODE_CURSOR_UP}},
	{ImGuiKey_DownArrow,	{BBC_KEYCODE_CURSOR_DOWN, BBC_KEYCODE_CURSOR_DOWN}},
	{ImGuiKey_CapsLock,		{BBC_KEYCODE_CAPS_LOCK, BBC_KEYCODE_CAPS_LOCK}},
	{ImGuiKey_Apostrophe,	{'\'', '@'}},
	{ImGuiKey_Comma,		{',', '<'}},
	{ImGuiKey_Minus,		{'-', '_'}},
	{ImGuiKey_Period,		{'.', '>'}},
	{ImGuiKey_Slash,		{'/', '?'}},
	{ImGuiKey_Semicolon,	{';', ':'}},
	{ImGuiKey_Equal,		{'=', '+'}},
	{ImGuiKey_LeftBracket,	{'[', '{'}},
	{ImGuiKey_Backslash,	{'\\', '|'}},
	{ImGuiKey_RightBracket,	{']', '}'}},
	{ImGuiKey_GraveAccent,	{'`', '~'}},
};

int BBCKeyFromImGuiKey(ImGuiKey key)
{
    uint32_t bbcKey = 0;
    bool isShifted = ImGui::GetIO().KeyShift;

    if (key >= ImGuiKey_0 && key <= ImGuiKey_9)
    {
        if (isShifted)
        {
            // Handle shifted number keys (e.g., '!' for '1')
            switch (key)
            {
                case ImGuiKey_1: bbcKey = (uint8_t)'!'; break;
                case ImGuiKey_2: bbcKey = (uint8_t)'"'; break;
                case ImGuiKey_3: bbcKey = (uint8_t)'£'; break;
                case ImGuiKey_4: bbcKey = (uint8_t)'$'; break;
                case ImGuiKey_5: bbcKey = (uint8_t)'%'; break;
                case ImGuiKey_6: bbcKey = (uint8_t)'^'; break;
                case ImGuiKey_7: bbcKey = (uint8_t)'&'; break;
                case ImGuiKey_8: bbcKey = (uint8_t)'*'; break;
                case ImGuiKey_9: bbcKey = (uint8_t)'('; break;
                case ImGuiKey_0: bbcKey = (uint8_t)')'; break;
            }
        }
        else
        {
            bbcKey = '0' + (key - ImGuiKey_0);
        }
    }
    else if (key >= ImGuiKey_A && key <= ImGuiKey_Z)
    {
        bbcKey = isShifted ? 'A' + (key - ImGuiKey_A) : 'a' + (key - ImGuiKey_A);
    }
    else
    {
        auto keyIt = g_BBCKeysLUT.find(key);
        if (keyIt != g_BBCKeysLUT.end())
        {
            bbcKey = isShifted ? keyIt->second.Shifted : keyIt->second.NoShift;
        }
    }
    return bbcKey;
}