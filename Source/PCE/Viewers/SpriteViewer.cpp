#include "SpriteViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>

FSpriteViewer::FSpriteViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Sprites";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FSpriteViewer::Init()
{
	return true;
}

void FSpriteViewer::DrawUI()
{
#if 0
	GeargrafxCore* core = pPCEEmu->GetCore();
	HuC6270* huc6270 = core->GetHuC6270_1();
	u16* sat = huc6270->GetSAT();
	//GG_Runtime_Info runtime;
	//emu_get_runtime(runtime);

	ImVec4 cyan = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
	float scale = 4.0f;

	//ImGui::SetNextWindowPos(ImVec2(78.0f + ((vdc == 1) ? 0 : 64), 56.0f + ((vdc == 1) ? 0 : 34)), ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize(ImVec2(546, 500), ImGuiCond_FirstUseEver);

	ImGuiIO& io = ImGui::GetIO();

	ImGui::Columns(2, "spr", false);
	ImGui::SetColumnOffset(1, 180.0f);

	ImGui::BeginChild("sprites", ImVec2(0, 0.0f), ImGuiChildFlags_Border);
	bool window_hovered = ImGui::IsWindowHovered();

	ImVec2 p[64];

	for (int s = 0; s < 64; s++)
	{
			p[s] = ImGui::GetCursorScreenPos();

			u16 sprite_flags = sat[(s * 4) + 3] & 0xB98F;
			float fwidth = k_huc6270_sprite_width[(sprite_flags >> 8) & 0x01] * scale;
			float fheight = k_huc6270_sprite_height[(sprite_flags >> 12) & 0x03] * scale;
			float tex_h = fwidth / 32.0f / scale;
			float tex_v = fheight / 64.0f / scale;

			ImGui::Image((ImTextureID)(intptr_t)renderer_emu_debug_huc6270_sprites[vdc - 1][s], ImVec2(fwidth, fheight), ImVec2(0.0f, 0.0f), ImVec2(tex_h, tex_v));

			float mouse_x = io.MousePos.x - p[s].x;
			float mouse_y = io.MousePos.y - p[s].y;

			if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < fwidth) && (mouse_y >= 0.0f) && (mouse_y < fheight))
			{
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					draw_list->AddRect(ImVec2(p[s].x, p[s].y), ImVec2(p[s].x + fwidth, p[s].y + fheight), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 3.0f);
			}
	}

	ImGui::EndChild();

	ImGui::NextColumn();

	ImVec2 p_screen = ImGui::GetCursorScreenPos();

	float screen_scale = 1.0f;
	float tex_h = (float)runtime.screen_width / (float)(SYSTEM_TEXTURE_WIDTH);
	float tex_v = (float)runtime.screen_height / (float)(SYSTEM_TEXTURE_HEIGHT);

	ImGui::Image((ImTextureID)(intptr_t)renderer_emu_texture, ImVec2(runtime.screen_width * screen_scale, runtime.screen_height * screen_scale), ImVec2(0, 0), ImVec2(tex_h, tex_v));

	for (int s = 0; s < 64; s++)
	{
			float mouse_x = io.MousePos.x - p[s].x;
			float mouse_y = io.MousePos.y - p[s].y;
			u16 sprite_flags = sat[(s * 4) + 3] & 0xB98F;
			int width = k_huc6270_sprite_width[(sprite_flags >> 8) & 0x01];
			int height = k_huc6270_sprite_height[(sprite_flags >> 12) & 0x03];
			float fwidth = width * scale;
			float fheight = height * scale;

			if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < fwidth) && (mouse_y >= 0.0f) && (mouse_y < fheight))
			{
					int sprite_y = (sat[s * 4] & 0x03FF) + 3;
					int sprite_x = sat[(s * 4) + 1] & 0x03FF;
					u16 pattern = (sat[(s * 4) + 2] >> 1) & 0x03FF;

					bool h_flip = (sprite_flags & 0x0800) != 0;
					bool v_flip = (sprite_flags & 0x8000) != 0;

					int palette = sprite_flags & 0x0F;
					bool priority = (sprite_flags & 0x0080) != 0;

					float real_x = (float)(sprite_x - 32);
					float real_y = (float)(sprite_y - 64);

					float rectx_min = p_screen.x + (real_x * screen_scale);
					float rectx_max = p_screen.x + ((real_x + width) * screen_scale);
					float recty_min = p_screen.y + (real_y * screen_scale);
					float recty_max = p_screen.y + ((real_y + height) * screen_scale);

					rectx_min = fminf(fmaxf(rectx_min, p_screen.x), p_screen.x + (runtime.screen_width * screen_scale));
					rectx_max = fminf(fmaxf(rectx_max, p_screen.x), p_screen.x + (runtime.screen_width * screen_scale));
					recty_min = fminf(fmaxf(recty_min, p_screen.y), p_screen.y + (runtime.screen_height * screen_scale));
					recty_max = fminf(fmaxf(recty_max, p_screen.y), p_screen.y + (runtime.screen_height * screen_scale));

					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					draw_list->AddRect(ImVec2(rectx_min, recty_min), ImVec2(rectx_max, recty_max), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

					ImGui::NewLine();

					ImGui::TextColored(cyan, "DETAILS:");
					ImGui::Separator();
					ImGui::TextColored(violet, " SAT ENTRY:"); ImGui::SameLine();
					ImGui::Text("%d", s);

					ImGui::TextColored(violet, " SPRITE X: "); ImGui::SameLine();
					ImGui::Text("%03X (%d)", sprite_x, sprite_x);

					ImGui::TextColored(violet, " SPRITE Y: "); ImGui::SameLine();
					ImGui::Text("%03X (%d)", sprite_y, sprite_y);

					ImGui::TextColored(violet, " SIZE:     "); ImGui::SameLine();
					ImGui::Text("%dx%d", width, height);

					ImGui::TextColored(violet, " PATTERN:  "); ImGui::SameLine();
					ImGui::Text("%03X (%d)", pattern, pattern);

					ImGui::TextColored(violet, " VRAM ADDR:"); ImGui::SameLine();
					ImGui::Text("$%04X", pattern << 6);

					ImGui::TextColored(violet, " PALETTE:  "); ImGui::SameLine();
					ImGui::Text("%01X (%d)", palette, palette);

					ImGui::TextColored(violet, " H FLIP:   "); ImGui::SameLine();
					ImGui::TextColored(h_flip ? green : gray, "%s", h_flip ? "YES" : "NO ");

					ImGui::TextColored(violet, " V FLIP:   "); ImGui::SameLine();
					ImGui::TextColored(v_flip ? green : gray, "%s", v_flip ? "YES" : "NO ");

					ImGui::TextColored(violet, " PRIORITY: "); ImGui::SameLine();
					ImGui::TextColored(priority ? green : gray, "%s", priority ? "YES" : "NO ");

					if (ImGui::IsMouseClicked(0))
					{
							gui_debug_memory_goto((vdc == 1) ? MEMORY_EDITOR_VRAM_1 : MEMORY_EDITOR_VRAM_2, pattern << 6);
					}
			}
	}

	ImGui::Columns(1);
#endif
}
