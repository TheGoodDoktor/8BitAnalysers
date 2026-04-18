#include "BackgroundViewer.h"

#include <imgui.h>
#include <ImGuiSupport/ImGuiTexture.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>

FBackgroundViewer::FBackgroundViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Background";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FBackgroundViewer::Init()
{
	BackgroundBuffer = new u8[HUC6270_MAX_BACKGROUND_WIDTH * HUC6270_MAX_BACKGROUND_HEIGHT * 4];
	
	// todo use above numbers?
	BackgroundTexture = ImGui_CreateTextureRGBA(pPCEEmu->GetFrameBuffer(), 512, 512);

	return true;
}

static const ImVec4 cyan = ImVec4(0.10f, 0.90f, 0.90f, 1.0f);
static const ImVec4 dark_gray = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
static const float k_scale_levels[3] = { 1.0f, 1.5f, 2.0f };

void FBackgroundViewer::DrawUI()
{
	UpdateBackground();

	GeargrafxCore* core = pPCEEmu->GetCore();
	HuC6270* huc6270 = core->GetHuC6270_1();
	HuC6270::HuC6270_State* huc6270_state = huc6270->GetState();

	u16* vram = huc6270->GetVRAM();
	int screen_reg = (huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07;
	int screen_size_x = k_huc6270_screen_size_x[screen_reg];
	int screen_size_y = k_huc6270_screen_size_y[screen_reg];
	int bat_size = screen_size_x * screen_size_y;

	static bool show_grid = true;
	static int zoom = 2;
	ImVec4 grid_color = dark_gray;
	grid_color.w = 0.3f;
	float scale = k_scale_levels[zoom];
	float size_h = 8.0f * screen_size_x * scale;
	float size_v = 8.0f * screen_size_y * scale;
	float spacing_h = 8.0f * scale;
	float spacing_v = 8.0f * scale;
	float texture_size_h = 1024.0f;
	float texture_size_v = 512.0f;

	if (ImGui::BeginTable("regs", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoPadOuterX))
	{
		ImGui::TableSetupColumn("one", ImGuiTableColumnFlags_WidthFixed, 200.0f);
		ImGui::TableSetupColumn("two", ImGuiTableColumnFlags_WidthFixed);

		ImGui::TableNextColumn();

		ImGui::PushItemWidth(60.0f);
		ImGui::Combo("Zoom##zoom_bg", &zoom, "1x\0""1.5x\0""2x\0\0");
		ImGui::Checkbox("Show Grid##grid_bg", &show_grid);

		ImGui::TableNextColumn();

		ImGui::Text("ENABLED  ");

		ImGui::SameLine();
		//ImGui::Text(huc6270_state->R[HUC6270_REG_CR] & 0x0080 ? green : gray, "%s", huc6270_state->R[HUC6270_REG_CR] & 0x0080 ? "YES" : "NO");
		ImGui::Text("%s", huc6270_state->R[HUC6270_REG_CR] & 0x0080 ? "YES" : "NO");

		ImGui::SameLine();
		ImGui::Text("        SCREEN");

		ImGui::SameLine();
		ImGui::Text("%dx%d", k_huc6270_screen_size_x[(huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07], k_huc6270_screen_size_y[(huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07]);

		ImGui::Text("SCROLL X ");

		ImGui::SameLine();
		ImGui::Text("%02X (%03d)", huc6270_state->R[HUC6270_REG_BXR], huc6270_state->R[HUC6270_REG_BXR]);

		ImGui::Text("SCROLL Y ");

		ImGui::SameLine();
		ImGui::Text("%02X (%03d)", huc6270_state->R[HUC6270_REG_BYR], huc6270_state->R[HUC6270_REG_BYR]);

		ImGui::EndTable();
	}

	ImGui::Separator();


	if (ImGui::BeginChild("##bg", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav))
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImGui_UpdateTextureRGBA(BackgroundTexture, BackgroundBuffer);

		ImGui::Image(BackgroundTexture, ImVec2(size_h, size_v), ImVec2(0.0f, 0.0f), ImVec2(BufferWidth / texture_size_h, BufferHeight / texture_size_v));

		if (show_grid)
		{
			float x = p.x;
			for (int n = 0; n <= screen_size_x; n++)
			{
				draw_list->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + size_v), ImColor(grid_color), 1.0f);
				x += spacing_h;
			}

			float y = p.y;
			for (int n = 0; n <= screen_size_y; n++)
			{
				draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + size_h, y), ImColor(grid_color), 1.0f);
				y += spacing_v;
			}
		}

		if (ImGui::IsItemHovered())
		{
			ImVec2 mouse_pos = ImGui::GetMousePos();
			ImVec2 rel_pos = ImVec2((mouse_pos.x - p.x) / scale, (mouse_pos.y - p.y) / scale);
			int x = (int)(rel_pos.x / 8.0f);
			int y = (int)(rel_pos.y / 8.0f);
			int i = (screen_size_x * y) + x;
			if (i >= 0 && i < bat_size)
			{
				ImVec2 tile_pos = ImVec2(p.x + (x * 8.0f * scale), p.y + (y * 8.0f * scale));
				ImVec2 tile_size = ImVec2(8.0f * scale, 8.0f * scale);
				draw_list->AddRect(tile_pos, ImVec2(tile_pos.x + tile_size.x, tile_pos.y + tile_size.y), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

				u16 bat_entry = vram[i];
				int tile_index = bat_entry & 0x07FF;
				int color_table = (bat_entry >> 12) & 0x0F;

				ImGui::BeginTooltip();

				float tile_scale = 16.0f;
				float tile_width = 8.0f * tile_scale;
				float tile_height = 8.0f * tile_scale;
				float tile_uv_h = (i % screen_size_x) * 8.0f;
				float tile_uv_v = (i / screen_size_x) * 8.0f;

				ImGui::Image(BackgroundTexture, ImVec2(tile_width, tile_height), ImVec2(tile_uv_h / texture_size_h, tile_uv_v / texture_size_v), ImVec2((tile_uv_h + 8) / texture_size_h, (tile_uv_v + 8) / texture_size_v));

				ImGui::Text("TILE INDEX   ");
				ImGui::SameLine();
				ImGui::Text("%03X", tile_index);

				ImGui::Text("TILE ADDRESS ");
				ImGui::SameLine();
				ImGui::Text("%03X", tile_index * 16);

				ImGui::Text("COLOR TABLE  ");
				ImGui::SameLine();
				ImGui::Text("%01X", color_table);

				ImGui::EndTooltip();

				/*if (ImGui::IsMouseClicked(0))
				{
					gui_debug_memory_goto((vdc == 1) ? MEMORY_EDITOR_VRAM_1 : MEMORY_EDITOR_VRAM_2, tile_index * 16);
				}*/
			}
		}
	}

	ImGui::EndChild();
}

void FBackgroundViewer::UpdateBackground()
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();
	HuC6270* huc6270 = pCore->GetHuC6270_1();
	HuC6270::HuC6270_State* huc6270_state = huc6270->GetState();
	HuC6260* huc6260 = pCore->GetHuC6260();

	const u16* vram = huc6270->GetVRAM();
	const int screen_reg = (huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07;
	const int screen_size_x = k_huc6270_screen_size_x[screen_reg];
	const int screen_size_y = k_huc6270_screen_size_y[screen_reg];
	BufferWidth  = screen_size_x * 8;
	BufferHeight = screen_size_y * 8;

	const u16* colorTable = huc6260->GetColorTable();

	// Lookup table: expand 3-bit PCE colour component to 8-bit, avoiding per-pixel multiply/divide.
	static const u8 kExpand3to8[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };

	// Iterate tile-by-tile so BAT and tile data are fetched once per tile/row
	// rather than being recomputed for every pixel.
	for (int tile_y = 0; tile_y < screen_size_y; tile_y++)
	{
		for (int tile_x = 0; tile_x < screen_size_x; tile_x++)
		{
			const u16 bat_entry      = vram[tile_x + tile_y * screen_size_x];
			const int tile_data      = (bat_entry & 0x07FF) * 16;
			const int color_tbl_base = ((bat_entry >> 12) & 0x0F) * 16;

			u8* dst_row = BackgroundBuffer + (tile_y * 8 * BufferWidth + tile_x * 8) * 4;

			for (int row = 0; row < 8; row++, dst_row += BufferWidth * 4)
			{
				// Both VRAM words for this tile row fetched once for all 8 pixels.
				const u16 word_a = vram[tile_data + row];
				const u16 word_b = vram[tile_data + row + 8];
				const u8 byte1 = word_a & 0xFF;
				const u8 byte2 = word_a >> 8;
				const u8 byte3 = word_b & 0xFF;
				const u8 byte4 = word_b >> 8;

				u8* dst = dst_row;
				for (int col = 0; col < 8; col++, dst += 4)
				{
					const int shift = 7 - col;
					const int color = ((byte1 >> shift) & 1)        |
					                  (((byte2 >> shift) & 1) << 1) |
					                  (((byte3 >> shift) & 1) << 2) |
					                  (((byte4 >> shift) & 1) << 3);

					const u16 cv = colorTable[(color == 0 ? 0 : color_tbl_base) + color];
					dst[0] = kExpand3to8[(cv >> 3) & 0x07]; // red
					dst[1] = kExpand3to8[(cv >> 6) & 0x07]; // green
					dst[2] = kExpand3to8[cv & 0x07];         // blue
					dst[3] = 255;
				}
			}
		}
	}
}