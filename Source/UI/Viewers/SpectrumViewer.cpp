#include "SpectrumViewer.h"

#include <Shared/CodeAnalyser/CodeAnalyser.h>

#include <imgui.h>
#include "../SpectrumEmu.h"
#include "../SpectrumConstants.h"
#include <Shared/CodeAnalyser/UI/CodeAnalyserUI.h>
#include <UI/Viewers/GraphicsViewer.h>
#include "../GlobalConfig.h"

#define NOMINMAX // without this std::min and std::max fail to compile
#include <windows.h> // for VK_* key defines
#include <Shared/Util/Misc.h>

void DrawArrow(ImDrawList* dl, ImVec2 pos, bool bLeftDirection);

void FSpectrumViewer::Init(FSpectrumEmu* pEmu)
{
	pSpectrumEmu = pEmu;
	SetInputEventHandler(this);
}

void FSpectrumViewer::Draw()
{
	const FGlobalConfig& config = GetGlobalConfig();
	FCodeAnalysisState& codeAnalysis = pSpectrumEmu->CodeAnalysis;
	FCodeAnalysisViewState& viewState = codeAnalysis.GetFocussedViewState();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const int borderOffsetX = (320 - 256) / 2;
	const int borderOffsetY = (256 - 192) / 2;
	//ImGui::Text("Instructions this frame: %d \t(max:%d)", instructionsThisFrame,maxInst);
	bool bJustSelectedChar = false;
	ImGui::Image(pSpectrumEmu->Texture, ImVec2(320, 256));

	// Draw an indicator to show which scanline is being drawn
	if (config.bShowScanLineIndicator && pSpectrumEmu->UIZX.dbg.dbg.stopped)
	{
		// Compensate for the fact the border area on a real spectrum is bigger than the emulated spectrum.
		const int topScreenScanLine = pSpectrumEmu->ZXEmuState.top_border_scanlines - 32;
		int scanlineY = std::min(std::max(pSpectrumEmu->ZXEmuState.scanline_y - topScreenScanLine, 0), 256);
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddLine(ImVec2(pos.x + 4, pos.y + scanlineY), ImVec2(pos.x + 320 - 8, pos.y + scanlineY), 0x50ffffff);
		DrawArrow(dl, ImVec2(pos.x - 2, pos.y + scanlineY - 6), false);
		DrawArrow(dl, ImVec2(pos.x + 320 - 11, pos.y + scanlineY - 6), true);
	}
	
	// draw hovered address
	if (viewState.HighlightAddress != -1)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		if (viewState.HighlightAddress >= kScreenPixMemStart && viewState.HighlightAddress <= kScreenPixMemEnd)	// pixel
		{
			int xp, yp;
			GetScreenAddressCoords(viewState.HighlightAddress, xp, yp);

			const int rx = static_cast<int>(pos.x) + borderOffsetX + xp;
			const int ry = static_cast<int>(pos.y) + borderOffsetY + yp;
			dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + 8, (float)ry + 1), 0xffffffff);
		}

		if (viewState.HighlightAddress >= kScreenAttrMemStart && viewState.HighlightAddress <= kScreenAttrMemEnd)	// attributes
		{
			int xp, yp;
			GetAttribAddressCoords(viewState.HighlightAddress, xp, yp);

			const int rx = static_cast<int>(pos.x) + borderOffsetX + xp;
			const int ry = static_cast<int>(pos.y) + borderOffsetY + yp;
			dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + 8, (float)ry + 8), 0xffffffff);
		}
	}

	if (ImGui::IsItemHovered())
	{
		const int xp = std::min(std::max((int)(io.MousePos.x - pos.x - borderOffsetX), 0), 255);
		const int yp = std::min(std::max((int)(io.MousePos.y - pos.y - borderOffsetY), 0), 191);

		const uint16_t scrPixAddress = GetScreenPixMemoryAddress(xp, yp);
		const uint16_t scrAttrAddress = GetScreenAttrMemoryAddress(xp, yp);

		if (scrPixAddress != 0)
		{
			ImDrawList* dl = ImGui::GetWindowDrawList();
			const int rx = static_cast<int>(pos.x) + borderOffsetX + (xp & ~0x7);
			const int ry = static_cast<int>(pos.y) + borderOffsetY + (yp & ~0x7);
			dl->AddRect(ImVec2((float)rx, (float)ry), ImVec2((float)rx + 8, (float)ry + 8), 0xffffffff);
			ImGui::BeginTooltip();
			ImGui::Text("Screen Pos (%d,%d)", xp, yp);
			ImGui::Text("Pixel: %s, Attr: %s", NumStr(scrPixAddress), NumStr(scrAttrAddress));

			const uint16_t lastPixWriter = codeAnalysis.GetLastWriterForAddress(scrPixAddress);
			const uint16_t lastAttrWriter = codeAnalysis.GetLastWriterForAddress(scrAttrAddress);
			ImGui::Text("Pixel Writer: ");
			ImGui::SameLine();
			DrawCodeAddress(codeAnalysis, viewState, lastPixWriter);
			ImGui::Text("Attribute Writer: ");
			ImGui::SameLine();
			DrawCodeAddress(codeAnalysis, viewState, lastAttrWriter);
			{
				//ImGui::Text("Image: ");
				//const float line_height = ImGui::GetTextLineHeight();
				const float rectSize = 10;
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				const float startPos = pos.x;
				//pos.y -= rectSize + 2;

				for (int byte = 0; byte < 8; byte++)
				{
					const uint8_t val = pSpectrumEmu->ReadByte(GetScreenPixMemoryAddress(xp & ~0x7, (yp & ~0x7) + byte));

					for (int bit = 7; bit >= 0; bit--)
					{
						const ImVec2 rectMin(pos.x, pos.y);
						const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
						if (val & (1 << bit))
							dl->AddRectFilled(rectMin, rectMax, 0xffffffff);
						else
							dl->AddRect(rectMin, rectMax, 0xffffffff);

						pos.x += rectSize;
					}

					pos.x = startPos;
					pos.y += rectSize;
				}

				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
			}
			ImGui::EndTooltip();
			//ImGui::Text("Pixel Writer: %04X, Attrib Writer: %04X", lastPixWriter, lastAttrWriter);

			
			if (ImGui::IsMouseClicked(0))
			{
				bScreenCharSelected = true;
				SelectedCharX = rx;
				SelectedCharY = ry;
				SelectPixAddr = scrPixAddress;
				SelectAttrAddr = scrAttrAddress;

				// store pixel data for selected character
				for (int charLine = 0; charLine < 8; charLine++)
					CharData[charLine] = pSpectrumEmu->ReadByte( GetScreenPixMemoryAddress(xp & ~0x7, (yp & ~0x7) + charLine));
				CharDataFound = codeAnalysis.FindMemoryPattern(CharData, 8, 0, FoundCharDataAddress);
				bJustSelectedChar = true;
			}

			if (ImGui::IsMouseClicked(1))
			{
				bScreenCharSelected = false;
				bJustSelectedChar = false;
			}

			if (ImGui::IsMouseDoubleClicked(0))
				CodeAnalyserGoToAddress(viewState, lastPixWriter);
			if (ImGui::IsMouseDoubleClicked(1))
				CodeAnalyserGoToAddress(viewState, lastAttrWriter);
		}

	}

	if (bScreenCharSelected == true)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 col = 0xffffffff;	// TODO: pulse
		dl->AddRect(ImVec2((float)SelectedCharX, (float)SelectedCharY), ImVec2((float)SelectedCharX + 8, (float)SelectedCharY + 8), col);

		ImGui::Text("Pixel Char Address: %s", NumStr(SelectPixAddr));
		//ImGui::SameLine();
		DrawAddressLabel(codeAnalysis, viewState, SelectPixAddr);
		ImGui::Text("Attribute Address: %s", NumStr(SelectAttrAddr));
		//ImGui::SameLine();
		DrawAddressLabel(codeAnalysis, viewState, SelectAttrAddr);

		if (CharDataFound)
		{
			ImGui::Text("Found at: %s", NumStr(FoundCharDataAddress));
			DrawAddressLabel(codeAnalysis, viewState, FoundCharDataAddress);
			//ImGui::SameLine();
			bool bShowInGfxView = ImGui::Button("Show in GFX View");
			ImGui::SameLine();
			ImGui::Checkbox("Wrap", &bCharSearchWrap);
			ImGui::SameLine();
			if (ImGui::Button("Format as Bitmap"))
			{
				FDataFormattingOptions formattingOptions;
				formattingOptions.StartAddress = FoundCharDataAddress;
				formattingOptions.ItemSize = 1;
				formattingOptions.NoItems = 8;
				formattingOptions.DataType = DataType::Bitmap;

				FormatData(codeAnalysis, formattingOptions);
				CodeAnalyserGoToAddress(viewState, FoundCharDataAddress, false);
			}

			if (bShowInGfxView)
			{
				if (!bJustSelectedChar)
				{
					bool bFound = codeAnalysis.FindMemoryPattern(CharData, 8, FoundCharDataAddress+8, FoundCharDataAddress);
					
					// If we didn't find anything, then wraparound and look from the start of ram.
					if (!bFound && bCharSearchWrap)
						CharDataFound = codeAnalysis.FindMemoryPattern(CharData, 8, 0, FoundCharDataAddress);
				}

				pSpectrumEmu->GraphicsViewerGoToAddress(FoundCharDataAddress);
			}
		}
	}

	// This is experimental
	if (ImGui::Button("Find and format all on-screen chars"))
	{
		uint8_t charData[8];

		for (int yChar = 0; yChar < 24; yChar++)
		{
			for (int xChar = 0; xChar < 32; xChar++)
			{
				const int xp = xChar * 8;
				const int yp = yChar * 8;
				bool bContainsBits = false;

				// store pixel data for selected character
				for (int charLine = 0; charLine < 8; charLine++)
				{
					charData[charLine] = pSpectrumEmu->ReadByte(GetScreenPixMemoryAddress(xp, yp + charLine));
					if (charData[charLine] != 0)
						bContainsBits = true;
				}

				if (bContainsBits)
				{
					uint16_t foundCharDataAddress;

					if (codeAnalysis.FindMemoryPattern(charData, 8, 0x5800, foundCharDataAddress))
					{
						const FCodeInfo* pCodeInfo = codeAnalysis.GetCodeInfoForAddress(foundCharDataAddress);
						if (pCodeInfo == nullptr || pCodeInfo->bDisabled)
						{
							FDataFormattingOptions formattingOptions;
							formattingOptions.StartAddress = foundCharDataAddress;
							formattingOptions.ItemSize = 1;
							formattingOptions.NoItems = 8;
							formattingOptions.DataType = DataType::Bitmap;

							FormatData(codeAnalysis, formattingOptions);
						}
					}
				}

			}
		}

		
	}

	ImGui::SliderFloat("Speed Scale", &pSpectrumEmu->ExecSpeedScale, 0.0f, 2.0f);
	//ImGui::SameLine();
	if (ImGui::Button("Reset"))
		pSpectrumEmu->ExecSpeedScale = 1.0f;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (pSpectrumEmu->bHasInterruptHandler)
	{
		ImGui::Text("IM2 Interrupt Handler at: %s", NumStr(pSpectrumEmu->InterruptHandlerAddress));
		DrawAddressLabel(codeAnalysis, viewState, pSpectrumEmu->InterruptHandlerAddress);
	}
	
	bWindowFocused = ImGui::IsWindowFocused();
}

int GetSpectrumKeyFromKeyCode(int keyCode)
{
	int speccyKey = 0;

	if (keyCode >= '0' && keyCode <= '9')
	{
		// pressed 0-9
		speccyKey = keyCode;
	}
	else if (keyCode >= 'A' && keyCode <= 'Z')
	{
		// pressed a-z
		speccyKey = keyCode + 0x20;
	}
	else if (keyCode == VK_SPACE)
	{
		speccyKey = ' '; 
	}
	else if (keyCode == VK_RETURN)
	{
		speccyKey = 0xd;
	}
	else if (keyCode == VK_CONTROL)
	{
		// symbol-shift
		speccyKey = 0xf;
	}
	else if (keyCode == VK_SHIFT)
	{
		// caps-shift
		speccyKey = 0xe;
	}
	else if (keyCode == VK_BACK)
	{
		// delete (shift and 0)
		speccyKey = 0xc;
	}
	return speccyKey;
	
	return 0;
}

void FSpectrumViewer::OnKeyUp(int keyCode) 
{
	zx_key_up(&pSpectrumEmu->ZXEmuState, GetSpectrumKeyFromKeyCode(keyCode));
}


void FSpectrumViewer::OnKeyDown(int keyCode) 
{
	if (bWindowFocused)
	{
		zx_key_down(&pSpectrumEmu->ZXEmuState, GetSpectrumKeyFromKeyCode(keyCode));
	}
}

void FSpectrumViewer::OnGamepadUpdated(int mask)
{
	if (zx_joystick_type(&pSpectrumEmu->ZXEmuState) != ZX_JOYSTICKTYPE_NONE)
	{
		zx_joystick(&pSpectrumEmu->ZXEmuState, mask);
	}
}

void FSpectrumViewer::OnChar(int charCode) 
{
}

void DrawArrow(ImDrawList* dl, ImVec2 pos, bool bLeftDirection)
{
	ImVec2 arrowPos = ImVec2(pos.x, pos.y);
	const float h = 13.f;
	float r = h * 0.40f;
	ImVec2 center = ImVec2(arrowPos.x + h * 0.5f, arrowPos.y + h * 0.5f);
	ImVec2 a, b, c;
	if (bLeftDirection) r = -r;
	a = ImVec2(+0.750f * r, +0.000f * r);
	b = ImVec2(-0.750f * r, +0.866f * r);
	c = ImVec2(-0.750f * r, -0.866f * r);
	dl->AddTriangleFilled(ImVec2(center.x + a.x, center.y + a.y), ImVec2(center.x + b.x, center.y + b.y), ImVec2(center.x + c.x, center.y + c.y), 0xffffffff);
}