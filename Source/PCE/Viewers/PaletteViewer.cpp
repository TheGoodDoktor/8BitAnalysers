#include "PaletteViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>
#include <ImGuiSupport/ImGuiScaling.h>
#include "Util/GraphicsView.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

FPaletteViewer::FPaletteViewer(FEmuBase* pEmu)
: FViewerBase(pEmu)
{
	Name = "Palettes";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FPaletteViewer::Init()
{
	return true;
}

void FPaletteViewer::DrawUI()
{
	ImGui::SetNextWindowPos(ImVec2(59, 70), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(526, 400), ImGuiCond_FirstUseEver);

	if (ImGui::BeginTabBar("##palette_tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Active", NULL, ImGuiTabItemFlags_None))
		{
			ImGui::BeginChild("active_palettes", ImVec2(0, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::SeparatorText("Background");
			for (int p = 0; p < 16; p++)
			{
				const uint32_t* pPalette = GetPaletteFromPaletteNo(p);
				if (!pPalette)
					continue;
				ImGui::Text("BG %2d:", p);
				ImGui::SameLine();
				DrawPalette(pPalette, 16, ImGui::GetFrameHeight());
				ImGui::SameLine();
				char btnId[32];
				snprintf(btnId, sizeof(btnId), "+##bg_%d", p);
				if (ImGui::SmallButton(btnId))
					pPCEEmu->CreateUserPalette(p);
			}

			ImGui::SeparatorText("Sprites");
			for (int p = 16; p < 32; p++)
			{
				const uint32_t* pPalette = GetPaletteFromPaletteNo(p);
				if (!pPalette)
					continue;
				ImGui::Text("SP %2d:", p - 16);
				ImGui::SameLine();
				DrawPalette(pPalette, 16, ImGui::GetFrameHeight());
				ImGui::SameLine();
				char btnId[32];
				snprintf(btnId, sizeof(btnId), "+##sp_%d", p);
				if (ImGui::SmallButton(btnId))
					pPCEEmu->CreateUserPalette(p);
			}
#ifndef NDEBUG
			ImGui::SeparatorText("Debug");
			ImGui::Text("Num Palette Entries: %d", GetNoPaletteEntries());
#endif
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("User", NULL, ImGuiTabItemFlags_None))
		{
			ImGui::BeginChild("user_palettes", ImVec2(0, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

			const int numPalettes = GetNoPaletteEntries();
			int displayNo = 0;
			for (int p = 32; p < numPalettes; p++)
			{
				const FPaletteEntry* pEntry = GetPaletteEntry(p);
				if (!pEntry)
					continue;
				const uint32_t* pPalette = GetPaletteFromPaletteNo(p);
				if (!pPalette)
					continue;
				ImGui::Text("User %d (idx %d):", displayNo, p);
				ImGui::SameLine();
				DrawPalette(pPalette, pEntry->NoColours, ImGui::GetFrameHeight());
				displayNo++;
			}

			if (displayNo == 0)
				ImGui::TextDisabled("No user palettes. Use the '+' button in the Active tab to copy one.");

			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}
