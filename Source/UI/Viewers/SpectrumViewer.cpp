#include "SpectrumViewer.h"

#include <Shared/CodeAnalyser/CodeAnalyser.h>

#include <imgui.h>
#include <Speccy/Speccy.h>
#include "../SpeccyUI.h"
#include <Shared/CodeAnalyser/CodeAnalyserUI.h>
#include <UI/Viewers/GraphicsViewer.h>

void ReadSpeccyKeys(FSpeccy* pSpeccy);

void FSpectrumViewer::Draw()
{
	FCodeAnalysisState& codeAnalysis = pSpectrumEmu->CodeAnalysis;
	FSpeccy* pSpeccy = pSpectrumEmu->pSpeccy;

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	//ImGui::Text("Instructions this frame: %d \t(max:%d)", instructionsThisFrame,maxInst);
	ImGui::Image(pSpeccy->Texture, ImVec2(320, 256));
	if (ImGui::IsItemHovered())
	{
		const int borderOffsetX = (320 - 256) / 2;
		const int borderOffsetY = (256 - 192) / 2;
		const int xp = std::min(std::max((int)(io.MousePos.x - pos.x - borderOffsetX), 0), 255);
		const int yp = std::min(std::max((int)(io.MousePos.y - pos.y - borderOffsetY), 0), 191);

		const uint16_t scrPixAddress = GetScreenPixMemoryAddress(xp, yp);
		const uint16_t scrAttrAddress = GetScreenAttrMemoryAddress(xp, yp);

		if (scrPixAddress != 0)
		{
			ImDrawList* dl = ImGui::GetWindowDrawList();
			const int rx = static_cast<int>(pos.x) + borderOffsetX + (xp & ~0x7);
			const int ry = static_cast<int>(pos.y) + borderOffsetY + (yp & ~0x7);
			dl->AddRect(ImVec2(rx, ry), ImVec2(rx + 8, ry + 8), 0xffffffff);
			ImGui::BeginTooltip();
			ImGui::Text("Screen Pos (%d,%d)", xp, yp);
			ImGui::Text("Pixel: %04Xh, Attr: %04Xh", scrPixAddress, scrAttrAddress);

			const uint16_t lastPixWriter = codeAnalysis.GetLastWriterForAddress(scrPixAddress);
			const uint16_t lastAttrWriter = codeAnalysis.GetLastWriterForAddress(scrAttrAddress);
			ImGui::Text("Pixel Writer: ");
			ImGui::SameLine();
			DrawCodeAddress(codeAnalysis, lastPixWriter);
			ImGui::Text("Attribute Writer: ");
			ImGui::SameLine();
			DrawCodeAddress(codeAnalysis, lastAttrWriter);
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
				uint8_t charData[8];
				for (int charLine = 0; charLine < 8; charLine++)
					charData[charLine] = ReadSpeccyByte(pSpeccy, GetScreenPixMemoryAddress(xp & ~0x7, (yp & ~0x7) + charLine));
				CharDataFound = codeAnalysis.FindMemoryPattern(charData, 8, FoundCharDataAddress);
			}

			if (ImGui::IsMouseClicked(1))
				bScreenCharSelected = false;

			if (ImGui::IsMouseDoubleClicked(0))
				CodeAnalyserGoToAddress(codeAnalysis, lastPixWriter);
			if (ImGui::IsMouseDoubleClicked(1))
				CodeAnalyserGoToAddress(codeAnalysis, lastAttrWriter);
		}

	}

	if (bScreenCharSelected == true)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 col = 0xffffffff;	// TODO: pulse
		dl->AddRect(ImVec2(SelectedCharX, SelectedCharY), ImVec2(SelectedCharX + 8, SelectedCharY + 8), col);

		ImGui::Text("Pixel Char Address:");
		ImGui::SameLine();
		DrawAddressLabel(codeAnalysis, SelectPixAddr);
		ImGui::Text("Attribute Address:");
		ImGui::SameLine();
		DrawAddressLabel(codeAnalysis, SelectAttrAddr);

		if (CharDataFound)
		{
			ImGui::Text("Found at: ");
			DrawAddressLabel(codeAnalysis, FoundCharDataAddress);
			ImGui::SameLine();
			if (ImGui::Button("Show in GFX View"))
				GraphicsViewerGoToAddress(FoundCharDataAddress);
		}
	}

	ImGui::SliderFloat("Speed Scale", &pSpeccy->ExecSpeedScale, 0.0f, 1.0f);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	// read keys
	if (ImGui::IsWindowFocused())
	{
		ReadSpeccyKeys(pSpeccy);
	}
}



void ReadSpeccyKeys(FSpeccy* pSpeccy)
{
	ImGuiIO& io = ImGui::GetIO();
	for (int i = 0; i < 10; i++)
	{
		if (io.KeysDown[0x30 + i] == 1)
		{
			zx_key_down(&pSpeccy->CurrentState, '0' + i);
			zx_key_up(&pSpeccy->CurrentState, '0' + i);
		}
	}

	for (int i = 0; i < 26; i++)
	{
		if (io.KeysDown[0x41 + i] == 1)
		{
			zx_key_down(&pSpeccy->CurrentState, 'a' + i);
			zx_key_up(&pSpeccy->CurrentState, 'a' + i);
		}
	}
}
