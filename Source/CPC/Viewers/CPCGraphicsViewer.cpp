#include "CPCGraphicsViewer.h"


#include <Util/GraphicsView.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include <ImGuiSupport/ImGuiScaling.h>

#include "../CPCEmu.h"

bool FCPCGraphicsViewer::Init()
{
	pCPCEmu = (FCPCEmu*)pEmulator;
	FGraphicsViewer::Init();
	BitmapFormat = EBitmapFormat::ColMap2Bpp_CPC;

#if 0
	// test view - REMOVE
	pTestGraphicsView = new FGraphicsView(128, 128);
	pTestGraphicsView->Clear(0xfffff00);
#endif

	return true;
}

void FCPCGraphicsViewer::DrawUI(void)
{
	if (ImGui::BeginTabBar("GraphicsViewTabBar"))
	{
		if (ImGui::BeginTabItem("GFX"))
		{
			DrawCharacterGraphicsViewer();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Screen"))
		{
			DrawScreenViewer();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Palette"))
		{
			DrawPaletteViewer();
			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();
}

// Amstrad CPC specific implementation
void FCPCGraphicsViewer::DrawScreenViewer()
{
	UpdateScreenPixelImage();
	pScreenView->Draw();

#if 0
	const uint32_t* pPalette = pCPCEmu->Screen.GetCurrentPalette().GetData();
	if (1)
	{
		// 1943
		// mode 0
		const uint16_t spriteAddress = 0x585f;
		const uint8_t* ptr = pCPCEmu->GetMemPtr(spriteAddress);
		pTestGraphicsView->Draw4BppWideImageAt(ptr, 32, 32, 14, 21, pPalette);
		pTestGraphicsView->Draw();
	}
#endif
}

// get offset into screen ram for a given horizontal pixel line (scan line)
uint16_t FCPCGraphicsViewer::GetPixelLineOffset(int yPos)
{
	// todo: couldn't we use FCPCEmu::GetScreenMemoryAddress() instead?

	return ((yPos / CharacterHeight) * (ScreenWidth / 4)) + ((yPos % CharacterHeight) * 2048);
}

uint32_t FCPCGraphicsViewer::GetRGBValueForPixel(int yPos, int colourIndex, uint32_t heatMapCol) const
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

void FCPCGraphicsViewer::UpdateScreenPixelImage(void)
{
	const FCodeAnalysisState& state = GetCodeAnalysis();
	const float fontSize = ImGui::GetFontSize();

	// todo: deal with Bank being set
	const mc6845_t& crtc = pCPCEmu->CPCEmuState.crtc;

	if (ImGui::Button("Get From CRTC Registers"))
	{
		DisplayAddress = pCPCEmu->Screen.GetScreenAddrStart();
		WidthChars = crtc.h_displayed;
		HeightChars = crtc.v_displayed;
		ScreenMode = pCPCEmu->CPCEmuState.ga.video.mode;
		CharacterHeight = crtc.max_scanline_addr + 1;
	}

	if (ImGui::InputInt("Screen Mode", &ScreenMode, 1, 8, ImGuiInputTextFlags_CharsDecimal))
	{
		ScreenMode = std::min(std::max(ScreenMode, 0), 1);
	}

	DrawPaletteCombo("Palette", "Current", PaletteNo, ScreenMode == 0 ? 16 : 4);

	if (PaletteNo == -1)
	{
		pPaletteColours = (uint32_t*)pCPCEmu->Screen.GetCurrentPalette().GetData();
	}
	else
	{
		pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);
	}

	ImGui::Checkbox("Show Reads & Writes", &bShowReadsWrites);

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

void FCPCGraphicsViewer::DrawPaletteViewer()
{
	const uint32_t* pCurrentPalette = pCPCEmu->Screen.GetCurrentPalette().GetData();

	ImGui::Text("Current Palette: ");
	DrawPalette(pCurrentPalette, pCPCEmu->CPCEmuState.ga.video.mode == 0 ? 16 : 4, ImGui::GetTextLineHeight());
	ImGui::Separator();

	static int paletteIndex = -1;
	if (ImGui::Button("Store Current Palette"))
	{
		paletteIndex = GetPaletteNo(pCurrentPalette, pCPCEmu->CPCEmuState.ga.video.mode == 0 ? 16 : 4);
	}

	const float scale = ImGui_GetScaling();

	int numPalettes = GetNoPaletteEntries();
	ImGui::Text("Palettes Stored: %d", GetNoPaletteEntries());

	for (int p = 0; p < numPalettes; p++)
	{
		if (const FPaletteEntry* pEntry = GetPaletteEntry(p))
		{
			const uint32_t* palette = GetPaletteFromPaletteNo(p);
			ImGui::Text("%02d: ", p);
			ImGui::SameLine();
			DrawPalette(palette, pEntry->NoColours, ImGui::GetTextLineHeight());
		}
	}
}

const uint32_t* FCPCGraphicsViewer::GetCurrentPalette() const 
{ 
	return pCPCEmu->Screen.GetCurrentPalette().GetData();
}
