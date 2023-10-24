#include "CPCGraphicsViewer.h"


#include <Util/GraphicsView.h>
#include <CodeAnalyser/CodeAnalyser.h>

#include "../CPCEmu.h"

void FCPCGraphicsViewer::Init(FCodeAnalysisState* pCodeAnalysis, FCpcEmu* pEmu)
{
	pCpcEmu = pEmu;
	FGraphicsViewer::Init(pCodeAnalysis);
}

// Amstrad CPC specific implementation
void FCPCGraphicsViewer::DrawScreenViewer()
{
	UpdateScreenPixelImage();
	pScreenView->Draw();
}

// get offset into screen ram for a given horizontal pixel line (scan line)
uint16_t FCPCGraphicsViewer::GetPixelLineOffset(int yPos)
{
	// todo: couldn't we use FCpcEmu::GetScreenMemoryAddress() instead?

	return ((yPos / CharacterHeight) * (ScreenWidth / 4)) + ((yPos % CharacterHeight) * 2048);
}

uint32_t FCPCGraphicsViewer::GetRGBValueForPixel(int yPos, int colourIndex, uint32_t heatMapCol) const
{
	const ImColor colour = pCpcEmu->Screen.GetPaletteForYPos(yPos).GetColour(colourIndex);
	
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

void FCPCGraphicsViewer::UpdateScreenPixelImage(void)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	const float fontSize = ImGui::GetFontSize();

	// todo: deal with Bank being set
	const mc6845_t& crtc = pCpcEmu->CpcEmuState.crtc;

	if (ImGui::Button("Set From CRTC Registers"))
	{
		DisplayAddress = pCpcEmu->Screen.GetScreenAddrStart();
		WidthChars = crtc.h_displayed;
		HeightChars = crtc.v_displayed;
		ScreenMode = pCpcEmu->CpcEmuState.ga.video.mode;
		CharacterHeight = crtc.max_scanline_addr + 1;
	}

	ImGui::PushItemWidth(fontSize * 10.0f);

	if (GetNumberDisplayMode() == ENumberDisplayMode::Decimal)
	{
		ImGui::InputInt("Address", &DisplayAddress, 1, 8, ImGuiInputTextFlags_CharsDecimal);
	}
	else
	{
		ImGui::InputInt("Address", &DisplayAddress, 1, 8, ImGuiInputTextFlags_CharsHexadecimal);
	}

	if (ImGui::InputInt("Width", &WidthChars, 1, 8, ImGuiInputTextFlags_CharsDecimal))
	{
		WidthChars = std::min(std::max(WidthChars, 0), 48);
	}

	if (ImGui::InputInt("Height", &HeightChars, 1, 8, ImGuiInputTextFlags_CharsDecimal))
	{
		HeightChars = std::min(std::max(HeightChars, 0), 35);
	}

	if (ImGui::InputInt("Screen Mode", &ScreenMode, 1, 8, ImGuiInputTextFlags_CharsDecimal))
	{
		ScreenMode = std::min(std::max(ScreenMode, 0), 1);
	}

	if (ImGui::InputInt("Character Height", &CharacterHeight, 1, 8, ImGuiInputTextFlags_CharsDecimal))
	{
		CharacterHeight = std::min(std::max(CharacterHeight, 1), 8);
	}
	ImGui::PopItemWidth();

	const int16_t startOffset = DisplayAddress & 0x3fff;
	const int16_t startBankId = Bank == -1 ? state.GetBankFromAddress(DisplayAddress) : Bank;
	const FCodeAnalysisBank* pBank = state.GetBank(startBankId);

	const int lastScreenWidth = ScreenWidth;
	const int lastScreenHeight = ScreenHeight;
	ScreenWidth = WidthChars * 8;
	ScreenHeight = HeightChars * CharacterHeight;

	ScreenHeight = std::min(ScreenHeight, AM40010_DISPLAY_HEIGHT);
	ScreenWidth = std::min(ScreenWidth, AM40010_DISPLAY_WIDTH >> 1);

	if (ScreenWidth != lastScreenWidth || ScreenHeight != lastScreenHeight)
	{
		// temp. recreate the screen view
		// todo: need to create this up front. 
		delete pScreenView;
		pScreenView = new FGraphicsView(ScreenWidth, ScreenHeight);
	}

#if 0
	// clear pixel buffer with single colour
	uint32_t* pPixBufAddr = pScreenView->GetPixelBuffer();
	for (int i = 0; i < ScreenWidth * ScreenHeight; i++)
	{
		*pPixBufAddr = 0xffff00ff;
		pPixBufAddr++;
	}
#endif

	for (int y = 0; y < ScreenHeight; y++)
	{
		uint16_t curLineOffset = startOffset + GetPixelLineOffset(y);

		const int bankSize = pBank->GetSizeBytes();
		if (curLineOffset >= bankSize)
		{
			// Deal with screen memory wrapping around
			curLineOffset -= bankSize;
		}

		// get pointer to start of line in pixel buffer
		uint32_t* pPixBufAddr = pScreenView->GetPixelBuffer() + (y * ScreenWidth);

		const int pixelsPerByte = ScreenMode == 0 ? 2 : 4;
		const int bytesPerLine = WidthChars * 2;
		for (int b = 0; b < bytesPerLine; b++)
		{
			const uint8_t val = pBank->Memory[curLineOffset];
			const FCodeAnalysisPage& page = pBank->Pages[curLineOffset >> 10];
			const uint32_t heatMapCol = GetHeatmapColourForMemoryAddress(page, curLineOffset, state.CurrentFrameNo, HeatmapThreshold);

			for (int p = 0; p < pixelsPerByte; p++)
			{
				const int colourIndex = GetHWColourIndexForPixel(val, p, ScreenMode);
				const ImU32 pixelColour = GetRGBValueForPixel(y, colourIndex, heatMapCol);

				*pPixBufAddr = pixelColour;
				pPixBufAddr++;

				if (ScreenMode == 0)
				{
					*pPixBufAddr = pixelColour;
					pPixBufAddr++;
				}
			}
			curLineOffset++;
			if (curLineOffset >= bankSize)
			{
				curLineOffset -= bankSize;
			}
		}
	}
}