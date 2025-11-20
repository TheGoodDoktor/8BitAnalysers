#include "PaletteViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>
#include <ImGuiSupport/ImGuiScaling.h>
#include "Util/GraphicsView.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

ImVec4 Convert333PaletteToFloat(u16 color)
{
	ImVec4 ret;
	ret.w = 0;
	ret.z = (1.0f / 7.0f) * (color & 0x7);
	ret.x = (1.0f / 7.0f) * ((color >> 3) & 0x7);
	ret.y = (1.0f / 7.0f) * ((color >> 6) & 0x7);
	return ret;
}

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

	GeargrafxCore* core = pPCEEmu->GetCore();
	HuC6260* huc6260 = core->GetHuC6260();
	u16* color_table = huc6260->GetColorTable();

	if (ImGui::BeginTabBar("##palette_tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Background", NULL, ImGuiTabItemFlags_None))
		{
			ImGui::BeginChild("background_palettes", ImVec2(0, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::NewLine();

			for (int row = 0; row < 16; row++)
			{
				for (int col = 0; col < 16; col++)
				{
					int i = row * 16 + col;
					if (col == 0)
					{
						ImGui::Text("%03X:", i); ImGui::SameLine();
					}

					u16 color = color_table[i];
					ImVec4 float_color = Convert333PaletteToFloat(color);
					char id[16];
					snprintf(id, 16, "##bg_pal_%d_%d", row, col);
					ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip);
					if (col != 15)
						ImGui::SameLine(0, 10);
				}

				/*ImGui::Text("     "); ImGui::SameLine(0, 0);

				for (int col = 0; col < 16; col++)
				{
					u16 color = color_table[row * 16 + col];
					u8 color_green = (color >> 6) & 0x07;
					u8 color_red = (color >> 3) & 0x07;
					u8 color_blue = color & 0x07;

					ImGui::Text("%01X", color_green); ImGui::SameLine(0, 0);
					ImGui::Text("%01X", color_red); ImGui::SameLine(0, 0);
					ImGui::Text("%01X", color_blue); ImGui::SameLine();
				}

				ImGui::NewLine();*/
			}

			ImGui::NewLine();

			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Sprites", NULL, ImGuiTabItemFlags_None))
		{
			ImGui::BeginChild("sprite_palettes", ImVec2(0, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::NewLine();

			for (int row = 16; row < 32; row++)
			{
				for (int col = 0; col < 16; col++)
				{
					int i = row * 16 + col;
					if (col == 0)
					{
						ImGui::Text("%03X:", i); ImGui::SameLine();
					}

					u16 color = color_table[i];
					ImVec4 float_color = Convert333PaletteToFloat(color);
					char id[16];
					snprintf(id, 16, "##spr_pal_%d_%d", row, col);
					ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip);
					if (col != 15)
						ImGui::SameLine(0, 10);
				}

				/*ImGui::Text("     "); ImGui::SameLine(0, 0);

				for (int col = 0; col < 16; col++)
				{
					u16 color = color_table[row * 16 + col];
					u8 color_green = (color >> 6) & 0x07;
					u8 color_red = (color >> 3) & 0x07;
					u8 color_blue = color & 0x07;

					ImGui::Text("%01X", color_green); ImGui::SameLine(0, 0);
					ImGui::Text("%01X", color_red); ImGui::SameLine(0, 0);
					ImGui::Text("%01X", color_blue); ImGui::SameLine();
				}

				ImGui::NewLine();*/
			}

			ImGui::NewLine();

			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("All Palettes", NULL, ImGuiTabItemFlags_None))
		{
			ImGui::BeginChild("all_palettes", ImVec2(0, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

			const float scale = ImGui_GetScaling();

			int numPalettes = GetNoPaletteEntries();
			ImGui::Text("Palettes Stored: %d", GetNoPaletteEntries());

			for (int p = 0; p < numPalettes; p++)
			{
				if (p == 0)
					ImGui::SeparatorText("Background");
				if (p == 16)
					ImGui::SeparatorText("Sprites");

				if (const FPaletteEntry* pEntry = GetPaletteEntry(p))
				{
					const uint32_t* palette = GetPaletteFromPaletteNo(p);
					ImGui::Text("%02d: ", p);
					ImGui::SameLine();
					DrawPalette(palette, pEntry->NoColours, ImGui::GetFrameHeight());
				}
			}

			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}
