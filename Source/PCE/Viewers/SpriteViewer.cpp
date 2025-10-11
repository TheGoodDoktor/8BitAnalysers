#include "SpriteViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>

#include <ImGuiSupport/ImGuiTexture.h>

FSpriteViewer::FSpriteViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Sprites";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FSpriteViewer::Init()
{
	ResetScreenTexture();

	for (int s = 0; s < kNumSprites; s++)
	{
		SpriteWidths[s] = 16;
		SpriteHeights[s] = 16;
		SpriteBuffers[s] = new u8[HUC6270_MAX_SPRITE_WIDTH * HUC6270_MAX_SPRITE_HEIGHT * 4];

		for (int j = 0; j < HUC6270_MAX_SPRITE_WIDTH * HUC6270_MAX_SPRITE_HEIGHT * 4; j++)
			SpriteBuffers[s][j] = 0;
	}

	return true;
}

void FSpriteViewer::ResetScreenTexture()
{
	GG_Runtime_Info info;
	pPCEEmu->GetCore()->GetRuntimeInfo(info);

	if (ScreenTexture && info.screen_height == TextureHeight && info.screen_width == TextureWidth)
		return;

	TextureWidth = info.screen_width;
	TextureHeight = info.screen_height;

	if (ScreenTexture != nullptr)
		ImGui_FreeTexture(ScreenTexture);

	ScreenTexture = ImGui_CreateTextureRGBA(pPCEEmu->GetFrameBuffer(), TextureWidth, TextureHeight);

	for (int s = 0; s < 64; s++)
	{
		SpriteTextures[s] = ImGui_CreateTextureRGBA(pPCEEmu->GetFrameBuffer(), HUC6270_MAX_SPRITE_WIDTH, HUC6270_MAX_SPRITE_HEIGHT);
	}
}

void FSpriteViewer::UpdateSpriteBuffers()
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();

	HuC6260* huc6260 = pCore->GetHuC6260();
	HuC6270* huc6270 = pCore->GetHuC6270_1();
	u16* vram = huc6270->GetVRAM();
	u16* sat = huc6270->GetSAT();
	u16* color_table = huc6260->GetColorTable();

	for (int i = 0; i < 64; i++)
	{
		int sprite_offset = i << 2;
		u16 flags = sat[sprite_offset + 3] & 0xB98F;
		bool x_flip = (flags & 0x0800);
		bool y_flip = (flags & 0x8000);
		int palette = flags & 0x0F;
		int cgx = (flags >> 8) & 0x01;
		int cgy = (flags >> 12) & 0x03;
		int width = k_huc6270_sprite_width[cgx];
		int height = k_huc6270_sprite_height[cgy];
		u16 pattern = (sat[sprite_offset + 2] >> 1) & 0x3FF;
		pattern &= k_huc6270_sprite_mask_width[cgx];
		pattern &= k_huc6270_sprite_mask_height[cgy];
		u16 sprite_address = pattern << 6;
		bool mode1 = ((huc6270->GetState()->R[HUC6270_REG_MWR] >> 2) & 0x03) == 1;
		int mode1_offset = mode1 ? (sat[sprite_offset + 2] & 1) << 5 : 0;

		SpriteWidths[i] = width;
		SpriteHeights[i] = height;

		for (int y = 0; y < height; y++)
		{
			int flipped_y = y_flip ? (height - 1 - y) : y;
			int tile_y = flipped_y >> 4;
			int tile_line_offset = tile_y * 2 * 64;
			int offset_y = flipped_y & 0xF;
			u16 line_start = sprite_address + tile_line_offset + offset_y;

			for (int x = 0; x < width; x++)
			{
				int flipped_x = x_flip ? (width - 1 - x) : x;
				int tile_x = flipped_x >> 4;
				int tile_x_offset = tile_x * 64;
				int line = line_start + tile_x_offset + mode1_offset;

				u16 plane1 = vram[line + 0];
				u16 plane2 = vram[line + 16];
				u16 plane3 = mode1 ? 0 : vram[line + 32];
				u16 plane4 = mode1 ? 0 : vram[line + 48];

				int pixel_x = 15 - (flipped_x & 0xF);
				u16 pixel = ((plane1 >> pixel_x) & 0x01) | (((plane2 >> pixel_x) & 0x01) << 1) | (((plane3 >> pixel_x) & 0x01) << 2) | (((plane4 >> pixel_x) & 0x01) << 3);
				pixel |= (palette << 4);
				pixel |= 0x100;

				int color = color_table[pixel & 0x1FF];
				u8 green = ((color >> 6) & 0x07) * 255 / 7;
				u8 red = ((color >> 3) & 0x07) * 255 / 7;
				u8 blue = (color & 0x07) * 255 / 7;

				if (!(pixel & 0x0F))
				{
					red = 255;
					green = 0;
					blue = 255;
				}

				int pixel_index = ((y * width) + x) << 2;
				SpriteBuffers[i][pixel_index + 0] = red;
				SpriteBuffers[i][pixel_index + 1] = green;
				SpriteBuffers[i][pixel_index + 2] = blue;
				SpriteBuffers[i][pixel_index + 3] = 255;
			}
		}
	}
}

void FSpriteViewer::DrawUI()
{
	ResetScreenTexture();
	UpdateSpriteBuffers();
	
	GeargrafxCore* core = pPCEEmu->GetCore();
	HuC6270* huc6270 = core->GetHuC6270_1();
	u16* sat = huc6270->GetSAT();

	ImVec4 cyan = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
	
	const FGlobalConfig* pConfig = pPCEEmu->GetGlobalConfig();
	const float scale = (float)pConfig->ImageScale;
	//float scale = 2.0f;
	
	ImGuiIO& io = ImGui::GetIO();

	bool window_hovered = ImGui::IsWindowHovered();

	ImVec2 p[64];

	ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;
	ImGuiStyle& style = ImGui::GetStyle();
	const float cellMinHeight = ((float)(HUC6270_MAX_SPRITE_HEIGHT>>1) * scale) + style.CellPadding.y * 2.0f;

	if (ImGui::BeginTable("spritetable", 8, flags))
	{
		for (int s = 0; s < 64; s++)
		{
			if (s % 8 == 0)
				ImGui::TableNextRow(ImGuiTableRowFlags_None, cellMinHeight);

			ImGui::TableNextColumn();

			p[s] = ImGui::GetCursorScreenPos();

			u16 sprite_flags = sat[(s * 4) + 3] & 0xB98F;
			float fwidth = k_huc6270_sprite_width[(sprite_flags >> 8) & 0x01] * scale;
			float fheight = k_huc6270_sprite_height[(sprite_flags >> 12) & 0x03] * scale;
			float tex_h = fwidth / 32.0f / scale;
			float tex_v = fheight / 64.0f / scale;

			ImGui_UpdateTextureSubImageRGBA(SpriteTextures[s], SpriteBuffers[s], SpriteWidths[s], SpriteHeights[s]);

			ImGui::Image(SpriteTextures[s], ImVec2(fwidth, fheight), ImVec2(0.0f, 0.0f), ImVec2(tex_h, tex_v));

			float mouse_x = io.MousePos.x - p[s].x;
			float mouse_y = io.MousePos.y - p[s].y;

			if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < fwidth) && (mouse_y >= 0.0f) && (mouse_y < fheight))
			{
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				draw_list->AddRect(ImVec2(p[s].x, p[s].y), ImVec2(p[s].x + fwidth, p[s].y + fheight), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 3.0f);
			}
		}
		ImGui::EndTable();
	}

#if 0
	ImVec2 p_screen = ImGui::GetCursorScreenPos();

	float screen_scale = 1.0f;
	//float tex_h = (float)runtime.screen_width / (float)(SYSTEM_TEXTURE_WIDTH);
	//float tex_v = (float)runtime.screen_height / (float)(SYSTEM_TEXTURE_HEIGHT);

	//ImGui_UpdateTextureRGBA(ScreenTexture, pPCEEmu->GetFrameBuffer());

	//ImGui::Image(ScreenTexture, ImVec2(TextureWidth * screen_scale, TextureHeight * screen_scale), ImVec2(0, 0)/*, ImVec2(tex_h, tex_v)*/);

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

			rectx_min = fminf(fmaxf(rectx_min, p_screen.x), p_screen.x + (TextureWidth * screen_scale));
			rectx_max = fminf(fmaxf(rectx_max, p_screen.x), p_screen.x + (TextureWidth * screen_scale));
			recty_min = fminf(fmaxf(recty_min, p_screen.y), p_screen.y + (TextureHeight * screen_scale));
			recty_max = fminf(fmaxf(recty_max, p_screen.y), p_screen.y + (TextureHeight * screen_scale));

			//ImDrawList* draw_list = ImGui::GetWindowDrawList();
			//draw_list->AddRect(ImVec2(rectx_min, recty_min), ImVec2(rectx_max, recty_max), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

			ImGui::NewLine();

			ImGui::TextColored(cyan, "Details:");
			ImGui::Separator();
			ImGui::Text("Sat Entry:"); ImGui::SameLine();
			ImGui::Text("%d", s);

			ImGui::Text("Sprite X: "); ImGui::SameLine();
			ImGui::Text("%03X (%d)", sprite_x, sprite_x);

			ImGui::Text("Sprite Y: "); ImGui::SameLine();
			ImGui::Text("%03X (%d)", sprite_y, sprite_y);

			ImGui::Text("Size:     "); ImGui::SameLine();
			ImGui::Text("%dx%d", width, height);

			ImGui::Text("Pattern:  "); ImGui::SameLine();
			ImGui::Text("%03X (%d)", pattern, pattern);

			ImGui::Text("VRAM Addr:"); ImGui::SameLine();
			ImGui::Text("$%04X", pattern << 6);

			ImGui::Text("Palette:  "); ImGui::SameLine();
			ImGui::Text("%01X (%d)", palette, palette);

			ImGui::Text("H Flip:   "); ImGui::SameLine();
			ImGui::Text("%s", h_flip ? "YES" : "NO ");

			ImGui::Text("V Flip:   "); ImGui::SameLine();
			ImGui::Text("%s", v_flip ? "YES" : "NO ");

			ImGui::Text("Priority: "); ImGui::SameLine();
			ImGui::Text("%s", priority ? "YES" : "NO ");

			/*if (ImGui::IsMouseClicked(0))
			{
					gui_debug_memory_goto((vdc == 1) ? MEMORY_EDITOR_VRAM_1 : MEMORY_EDITOR_VRAM_2, pattern << 6);
			}*/
		}
	}

	//ImGui::Columns(1);
#endif
}
