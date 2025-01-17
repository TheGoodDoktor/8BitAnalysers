#include "BBCGraphicsViewer.h"
#include "../BBCEmulator.h"
#include "../BBCChipsImpl.h"
#include <Util/GraphicsView.h>

#include <imgui.h>
#include <CodeAnalyser/UI/UIColours.h>

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

void FBBCGraphicsViewer::DrawScreenViewer()
{
	ImGui::Text("TODO: Screen Viewer");

	const mc6845_t& crtc = pBBCEmu->GetBBC().crtc;

	//DisplayAddress = pCPCEmu->Screen.GetScreenAddrStart();
	int widthChars = crtc.h_displayed;
	int heightChars = crtc.v_displayed;
	//ScreenMode = pCPCEmu->CPCEmuState.ga.video.mode;
	int characterHeight = crtc.max_scanline_addr + 1;

	ImGui::Text("Screen width chars: %d x %d", widthChars, heightChars);
	ImGui::Text("Character height: %d", characterHeight);

	//UpdateScreenPixelImage();
	UpdateScreenTeletextImage();
	pScreenView->Draw();
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


void FBBCGraphicsViewer::UpdateScreenPixelImage(void)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();

	const int16_t startOffset = DisplayAddress & 0x3fff;
	const int16_t startBankId = Bank == -1 ? state.GetBankFromAddress(DisplayAddress) : Bank;
	const FCodeAnalysisBank* pBank = state.GetBank(startBankId);

	const int lastScreenWidth = ScreenWidth;
	const int lastScreenHeight = ScreenHeight;
	ScreenWidth = WidthChars * 8;
	ScreenHeight = HeightChars * CharacterHeight;

	if (ScreenWidth != lastScreenWidth || ScreenHeight != lastScreenHeight)
	{
		// temp. recreate the screen view
		// todo: need to create this up front. 
		delete pScreenView;
		pScreenView = new FGraphicsView(ScreenWidth, ScreenHeight);
	}

	const FCodeAnalysisViewState& viewState = pBBCEmu->GetCodeAnalysis().GetFocussedViewState();
	const uint32_t* pEnd = pScreenView->GetPixelBuffer() + (ScreenWidth * ScreenHeight);
	const int bitShift = ScreenMode == 0 ? 0x1 : 0x0;

	for (int l = 0; l < CharacterHeight; l++)
	{
		uint32_t* pCurPixBufAddr = pScreenView->GetPixelBuffer() + (ScreenWidth * l);
		int16_t curBnkOffset = startOffset + l * 2048;

		for (int c = 0; c < HeightChars; c++)
		{
			uint8_t screenByte = 0;
			uint32_t heatMapCol = 0;

			// Draw a single pixel row
			for (int x = 0; x < ScreenWidth; x++)
			{
				const int p = (x & 0x3);
				if ((p & 0x3) == 0)
				{
					screenByte = pBank->Memory[curBnkOffset];
					const FCodeAnalysisPage& page = pBank->Pages[curBnkOffset >> FCodeAnalysisPage::kPageShift];
					heatMapCol = GetHeatmapColourForMemoryAddress(page, curBnkOffset, state.CurrentFrameNo, HeatmapThreshold);

					curBnkOffset++;

					// Deal with crossing a 2k page boundary. 
					if ((curBnkOffset % 2048) == 0)
						curBnkOffset -= 2048;
				}

				ImU32 pixelColour = 0;
				if (viewState.HighlightAddress.IsValid())
				{
					if (viewState.HighlightAddress.Address == pBank->GetMappedAddress() + curBnkOffset)
					{
						// Flash this pixel if the address is highlighted in the code analysis view 
						pixelColour = Colours::GetFlashColour();
					}
				}
				if (!pixelColour)
				{
					const int colourIndex = GetHWColourIndexForPixel(screenByte, p >> bitShift, ScreenMode);
					pixelColour = GetRGBValueForPixel(colourIndex, heatMapCol);
				}
				assert(pCurPixBufAddr < pEnd);
				*pCurPixBufAddr = pixelColour;
				pCurPixBufAddr++;
			}

			pCurPixBufAddr += ScreenWidth * (CharacterHeight - 1);
		}
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

	for (int y = 0; y < HeightChars; y++)
	{
		for (int x = 0; x < WidthChars; x++)
		{
			int charCode = 0x41 - 0x20; // 'A' - TODO : get the actual character code

			// TODO: check for control character
			const uint16_t* pChar = mode7font[charCode];

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
