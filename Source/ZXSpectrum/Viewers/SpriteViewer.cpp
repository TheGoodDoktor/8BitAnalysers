#include "SpriteViewer.h"
#include "GraphicsViewer.h"
#include "../SpectrumEmu.h"
#include <algorithm>
#include "../GameConfig.h"
#include "ZXGraphicsView.h"
#include <Shared/Util/Misc.h>

// Sprites

void GenerateSpriteList(FSpriteDefList &spriteList, uint16_t startAddress, int count, int width, int height)
{
	spriteList.Sprites.clear();
	spriteList.BaseAddress = startAddress;
	spriteList.Width = width;
	spriteList.Height = height;

	uint16_t spriteAddress = startAddress;
	for (int i = 0; i < count; i++)
	{
		FSpriteDef spriteDef;

		spriteDef.Address = spriteAddress;
		spriteDef.Width = width;
		spriteDef.Height = height;

		spriteAddress += width * height * 8;	// 8 bytes per char
		spriteList.Sprites.push_back(spriteDef);
	}
}

// Generate sprite lists from configs
void GenerateSpriteListsFromConfig(FGraphicsViewerState &state, FGameConfig *pGameConfig)
{
	state.SpriteLists.clear();
	for (const auto &sprConfIt : pGameConfig->SpriteConfigs)
	{
		const FSpriteDefConfig& config = sprConfIt.second;
		FUISpriteList &sprites = state.SpriteLists[sprConfIt.first];
		GenerateSpriteList(sprites.SpriteList, config.BaseAddress, config.Count, config.Width, config.Height);
	}
}

void DrawSpriteOnGraphicsView(const FSpriteDef &spriteDef, int x, int y, FZXGraphicsView *pGraphicsView, FSpectrumEmu* pSpeccy)
{
	const uint8_t *pImage = pSpeccy->GetMemPtr(spriteDef.Address);
	pGraphicsView->DrawBitImage(pImage, 0, 0, spriteDef.Width, spriteDef.Height);
}

void DrawSpriteList(const FSpriteDefList &spriteList, int &selection, FZXGraphicsView *pGraphicsView, FSpectrumEmu*pSpeccy)
{
	ImGui::InputInt("SpriteNo", &selection, 1, 1);
	selection = std::min(std::max(selection, 0), (int)spriteList.Sprites.size() - 1);

	pGraphicsView->Clear(0);
	DrawSpriteOnGraphicsView(spriteList.Sprites[selection], 0, 0, pGraphicsView, pSpeccy);
}

void DrawSpriteListGUI(FGraphicsViewerState &state, FZXGraphicsView *pGraphicsView)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("DrawSpriteListGUIChild1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, window_flags);
	FUISpriteList *pSpriteList = nullptr;

	// TODO: put in structure?
	static int baseAddress = 0;
	static int w = 0;
	static int h = 0;
	static int count = 0;
	
	for (auto &spriteList : state.SpriteLists)
	{
		const bool bSelected = state.SelectedSpriteList == spriteList.first;
		if (bSelected)
		{
			pSpriteList = &spriteList.second;
		}
	
		if (ImGui::Selectable(spriteList.first.c_str(), bSelected))
		{
			state.SelectedSpriteList = spriteList.first;
			FSpriteDefList &spriteDefList = spriteList.second.SpriteList;

			// set up editable properties
			baseAddress = spriteDefList.BaseAddress;
			w = spriteDefList.Width;
			h = spriteDefList.Height;
			count = (int)spriteDefList.Sprites.size();
		}
	}

	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("DrawSpriteListGUIChild2", ImVec2(0, 0), false, window_flags);

	if (pSpriteList != nullptr)
	{
		FSpriteDefList &spriteDefList = pSpriteList->SpriteList;

		if (ImGui::CollapsingHeader("Generation"))
		{
			if (GetNumberDisplayMode() == ENumberDisplayMode::Decimal)
				ImGui::InputInt("BaseAddress", &baseAddress, 1, 8, ImGuiInputTextFlags_CharsDecimal);
			else
				ImGui::InputInt("BaseAddress", &baseAddress, 1, 8, ImGuiInputTextFlags_CharsHexadecimal);

			ImGui::InputInt("Width", &w);
			ImGui::InputInt("Height", &h);
			ImGui::InputInt("Count", &count);
			if (ImGui::Button("Regenerate"))
			{
				GenerateSpriteList(spriteDefList, baseAddress, count, w, h);
			}
		}
		DrawSpriteList(pSpriteList->SpriteList, pSpriteList->Selection, pGraphicsView, state.pEmu);
	}
	pGraphicsView->Draw();

	ImGui::EndChild();
}
