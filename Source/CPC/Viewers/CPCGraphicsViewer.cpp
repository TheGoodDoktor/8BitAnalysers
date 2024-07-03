#include "CPCGraphicsViewer.h"


#include <Util/GraphicsView.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "CodeAnalyser/UI/ComboBoxes.h"
#include <ImGuiSupport/ImGuiScaling.h>

#include "../CPCEmu.h"

void HelpMarker(const char* desc);

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
	const float fontSize = ImGui::GetFontSize();

	DrawPaletteCombo("Palette", "Current", PaletteNo, ScreenMode == 0 ? 16 : 4);

	if (PaletteNo == -1)
	{
		pPaletteColours = (uint32_t*)pCPCEmu->Screen.GetCurrentPalette().GetData();
}
	else
	{
		pPaletteColours = GetPaletteFromPaletteNo(PaletteNo);
	}

	ImGui::PushItemWidth(fontSize * 10.0f);

	ImGui::SeparatorText("Screen Properties");

	if (ImGui::InputInt("Screen Mode", &ScreenMode, 1, 8, ImGuiInputTextFlags_CharsDecimal))
	{
		ScreenMode = std::min(std::max(ScreenMode, 0), 1);
	}

	const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
	ImGui::InputInt("Address", &DisplayAddress, 1, 8, inputFlags);

	if (ImGui::Checkbox("Track Non-Active Screen Buffer Address", &bTrackNonActiveScrBufferAddress))
	{
		DisplayAddress = ScrAddrHistory[ScrAddrHistoryIndex ? 0 : 1];
	}
	ImGui::SameLine();
	HelpMarker("For use in games with a double buffered screen setup, this will automatically display the back buffer. Serves no purpose in games that use a static screen.");

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

	if (ImGui::Button("Set Properties From CRTC Registers"))
	{
		const mc6845_t& crtc = pCPCEmu->CPCEmuState.crtc;
		DisplayAddress = pCPCEmu->Screen.GetScreenAddrStart();
		WidthChars = crtc.h_displayed;
		HeightChars = crtc.v_displayed;
		ScreenMode = pCPCEmu->CPCEmuState.ga.video.mode;
		CharacterHeight = crtc.max_scanline_addr + 1;
	}

	ImGui::SeparatorText("Screen Address History");

	const bool bIdentifyFrontBuffer = ScrAddrHistory[0] != ScrAddrHistory[1];
	const uint16_t scrAddress = pCPCEmu->Screen.GetScreenAddrStart();

	ImGui::Text("%s", NumStr(ScrAddrHistory[0]));
	ImGui::SameLine();
	if (ImGui::Button("Set##1"))
	{
		DisplayAddress = ScrAddrHistory[0];
	}
	if (bIdentifyFrontBuffer && scrAddress == ScrAddrHistory[0])
	{
		ImGui::SameLine();
		ImGui::Text("<-- Front buffer");
	}

	ImGui::Text("%s", NumStr(ScrAddrHistory[1]));
	ImGui::SameLine();
	if (ImGui::Button("Set##2"))
	{
		DisplayAddress = ScrAddrHistory[1];
	}
	if (bIdentifyFrontBuffer && scrAddress == ScrAddrHistory[1])
	{
		ImGui::SameLine();
		ImGui::Text("<-- Front buffer");
	}

	ImGui::Separator();
	UpdateScreenPixelImage();
	pScreenView->Draw();

	ImGui::Checkbox("Show Reads & Writes", &bShowReadsWrites);
	FrameCounter++;

	/*if (ImGui::Button("write to screen pixels"))
	{
		for (int i = 0; i < 0xffff; i++)
		{
			if (pCPCEmu->Screen.IsScreenAddress(i))
			{
				pCPCEmu->GetCodeAnalysis().WriteByte(i, 0xf0);
			}
		}
	}*/


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

uint32_t FCPCGraphicsViewer::GetRGBValueForPixel(int colourIndex, uint32_t heatMapCol) const
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
#if 0
	// clear pixel buffer with single colour
	uint32_t* pPixBufAddr = pScreenView->GetPixelBuffer();
	for (int i = 0; i < ScreenWidth * ScreenHeight; i++)
	{
		*pPixBufAddr = 0xffff00ff;
		pPixBufAddr++;
	}
#endif

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

	const FCodeAnalysisViewState& viewState = pCPCEmu->GetCodeAnalysis().GetFocussedViewState();
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
						pixelColour = GetFlashColour();
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

void FCPCGraphicsViewer::OnScreenAddressChanged(uint16_t addr)
{
	if (ScrAddrHistory[ScrAddrHistoryIndex] == addr)
		return;

	if (bTrackNonActiveScrBufferAddress)
		DisplayAddress = ScrAddrHistory[ScrAddrHistoryIndex];

	ScrAddrHistoryIndex = ScrAddrHistoryIndex ? 0 : 1;
	ScrAddrHistory[ScrAddrHistoryIndex] = addr;
}

ImU32 FCPCGraphicsViewer::GetFlashColour() const
{
	// generate flash colour
	ImU32 flashCol = 0xff000000;
	const int flashCounter = FrameCounter >> 2;
	if (flashCounter & 1) flashCol |= 0xff << 0;
	if (flashCounter & 2) flashCol |= 0xff << 8;
	if (flashCounter & 4) flashCol |= 0xff << 16;
	return flashCol;
}