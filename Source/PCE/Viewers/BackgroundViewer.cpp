#include "BackgroundViewer.h"

#include <imgui.h>
#include <ImGuiSupport/ImGuiTexture.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>

#include "VRAMViewer.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

FBackgroundViewer::FBackgroundViewer(FEmuBase* pEmu)
: FViewerBase(pEmu)
{
	Name = "Background";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FBackgroundViewer::Init()
{
	return true;
}

static const ImVec4 cyan = ImVec4(0.10f, 0.90f, 0.90f, 1.0f);
static const ImVec4 dark_gray = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);

void FBackgroundViewer::DrawUI()
{
	const bool dirty = UpdateBackground();
	if (dirty && BackgroundTexture)
		ImGui_UpdateTextureRGBA(BackgroundTexture, BackgroundBuffer);

	GeargrafxCore* core = pPCEEmu->GetCore();
	HuC6270* huc6270 = core->GetHuC6270_1();
	HuC6270::HuC6270_State* huc6270_state = huc6270->GetState();
	FVRAMViewer* pVRAMViewer = pPCEEmu->GetVRAMViewer();
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	u16* vram = huc6270->GetVRAM();
	int screen_reg = (huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07;
	int screen_size_x = k_huc6270_screen_size_x[screen_reg];
	int screen_size_y = k_huc6270_screen_size_y[screen_reg];
	int bat_size = screen_size_x * screen_size_y;

	static bool show_grid = false;
	static bool show_write_activity = false;
	static int scale = 2;
	ImVec4 grid_color = dark_gray;
	grid_color.w = 0.3f;
	float size_h = 8.0f * screen_size_x * scale;
	float size_v = 8.0f * screen_size_y * scale;
	float spacing_h = 8.0f * scale;
	float spacing_v = 8.0f * scale;
	const float texture_size_h = (float)BufferWidth;
	const float texture_size_v = (float)BufferHeight;

	if (ImGui::BeginTable("regs", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoPadOuterX))
	{
		ImGui::TableSetupColumn("one", ImGuiTableColumnFlags_WidthFixed, 200.0f);
		ImGui::TableSetupColumn("two", ImGuiTableColumnFlags_WidthFixed);

		ImGui::TableNextColumn();

		ImGui::InputInt("Scale", &scale, 1, 1);
		scale = MAX(scale, 1);

		ImGui::Checkbox("Show Grid##grid_bg", &show_grid);
		ImGui::Checkbox("Write Activity##writeact_bg", &show_write_activity);

		ImGui::TableNextColumn();

		ImGui::Text("Enabled: %s", huc6270_state->R[HUC6270_REG_CR] & 0x0080 ? "YES" : "NO");
		ImGui::Text("Screen: %d x %d", k_huc6270_screen_size_x[(huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07], k_huc6270_screen_size_y[(huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07]);
		ImGui::Text("Scroll X: %s", NumStr(huc6270_state->R[HUC6270_REG_BXR]));
		ImGui::Text("Scroll Y: ", NumStr(huc6270_state->R[HUC6270_REG_BYR]));

		ImGui::EndTable();
	}

	ImGui::Separator();


	if (ImGui::BeginChild("##bg", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav))
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		if (BackgroundTexture)
			ImGui::Image(BackgroundTexture, ImVec2(size_h, size_v));

		if (show_write_activity && pVRAMViewer)
		{
			const int currentFrame = state.CurrentFrameNo;
			for (int tile_i = 0; tile_i < bat_size; tile_i++)
			{
				const FVRAMAccess& access = pVRAMViewer->GetVRAMAccess(tile_i);
				if (access.FrameLastWritten == -1)
					continue;

				const int tx = tile_i % screen_size_x;
				const int ty = tile_i / screen_size_x;
				const ImVec2 tile_min = ImVec2(p.x + tx * spacing_h, p.y + ty * spacing_v);
				const ImVec2 tile_max = ImVec2(tile_min.x + spacing_h, tile_min.y + spacing_v);

				const bool active = (currentFrame - access.FrameLastWritten) <= 4;
				const ImU32 col = active ? IM_COL32(255, 255, 0, 100) : IM_COL32(128, 128, 0, 50);
				draw_list->AddRectFilled(tile_min, tile_max, col);
			}
		}

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

				const FAddressRef batWriter = pVRAMViewer ? pVRAMViewer->GetVRAMAccess(i).LastWriter : FAddressRef();
				if (batWriter.IsValid() && ImGui::IsMouseDoubleClicked(0))
				{
					if (ImGui::GetIO().KeyShift)
						state.GetAltViewState().GoToAddress(batWriter, false);
					else
						viewState.GoToAddress(batWriter, false);
				}

				ImGui::BeginTooltip();

				float tile_scale = 16.0f;
				float tile_width = 8.0f * tile_scale;
				float tile_height = 8.0f * tile_scale;
				float tile_uv_h = (i % screen_size_x) * 8.0f;
				float tile_uv_v = (i / screen_size_x) * 8.0f;

				ImGui::Image(BackgroundTexture, ImVec2(tile_width, tile_height), ImVec2(tile_uv_h / texture_size_h, tile_uv_v / texture_size_v), ImVec2((tile_uv_h + 8) / texture_size_h, (tile_uv_v + 8) / texture_size_v));
				char buf[64];

				ImGuiTableFlags tblFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV;
				if (ImGui::BeginTable("##batdetails", 3, tblFlags))
				{
					ImGui::TableSetupColumn("Attribute",   ImGuiTableColumnFlags_WidthFixed,  100.0f);
					ImGui::TableSetupColumn("Value",       ImGuiTableColumnFlags_WidthFixed,   80.0f);
					ImGui::TableSetupColumn("Last Writer", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableHeadersRow();

					auto Row = [&](const char* attr, const char* val, const FAddressRef& writer)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(attr);
						ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(val);
						ImGui::TableSetColumnIndex(2);
						if (writer.IsValid())
							DrawAddressLabel(state, viewState, writer);
						else
							ImGui::TextDisabled("--");
					};

					snprintf(buf, sizeof(buf), "$%04X", i);
					Row("BAT Address", buf, FAddressRef());

					snprintf(buf, sizeof(buf), "$%03X", tile_index);
					Row("Tile Index", buf, batWriter);

					snprintf(buf, sizeof(buf), "$%04X", tile_index * 16);
					Row("VRAM Address", buf, batWriter);

					snprintf(buf, sizeof(buf), "$%X", color_table);
					Row("Color Table", buf, batWriter);

					ImGui::EndTable();
				}

				ImGui::EndTooltip();
			}
		}
	}

	ImGui::EndChild();
}

bool FBackgroundViewer::UpdateBackground()
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();
	HuC6270* huc6270 = pCore->GetHuC6270_1();
	HuC6270::HuC6270_State* huc6270_state = huc6270->GetState();
	HuC6260* huc6260 = pCore->GetHuC6260();
	FVRAMViewer* pVRAMViewer = pPCEEmu->GetVRAMViewer();
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	const u16* vram        = huc6270->GetVRAM();
	const int screen_reg   = (huc6270_state->R[HUC6270_REG_MWR] >> 4) & 0x07;
	const int screen_size_x = k_huc6270_screen_size_x[screen_reg];
	const int screen_size_y = k_huc6270_screen_size_y[screen_reg];
	const int bat_size      = screen_size_x * screen_size_y;
	const int new_width     = screen_size_x * 8;
	const int new_height    = screen_size_y * 8;
	const int currentFrame  = state.CurrentFrameNo;

	// Resize buffer and texture to the actual BAT pixel dimensions when they change.
	// This keeps the GPU upload to the minimum needed (e.g. 256x256 for a 32x32 BAT
	// instead of the fixed 1024x512 max).
	if (new_width != BufferWidth || new_height != BufferHeight)
	{
		BufferWidth  = new_width;
		BufferHeight = new_height;
		delete[] BackgroundBuffer;
		BackgroundBuffer = new u8[BufferWidth * BufferHeight * 4]();
		if (BackgroundTexture)
			ImGui_FreeTexture(BackgroundTexture);
		BackgroundTexture = ImGui_CreateTextureRGBA(BackgroundBuffer, BufferWidth, BufferHeight);
		ShadowBAT.assign(bat_size, 0xFFFF);
		LastRenderedFrame = -1;
	}

	// If CurrentFrameNo went backwards (game reset / new game loaded) our cached
	// LastRenderedFrame is stale — force a full re-render.
	if (currentFrame < LastRenderedFrame)
		LastRenderedFrame = -1;

	const u16* colorTable = huc6260->GetColorTable();

	// Palette changes are independent of VRAM writes, so track them separately.
	// Any change forces every tile to re-render with the new colours.
	if (memcmp(colorTable, ShadowColorTable, 512 * sizeof(u16)) != 0)
	{
		memcpy(ShadowColorTable, colorTable, 512 * sizeof(u16));
		LastRenderedFrame = -1;
	}

	// Build a 512-entry RGBA lookup so each pixel is one table read + one 32-bit write
	// instead of three kExpand3to8 lookups and four byte stores.
	static const u8 kExpand3to8[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
	uint32_t rgbaLUT[512];
	for (int i = 0; i < 512; i++)
	{
		const u16 cv = colorTable[i];
		const u32 r  = kExpand3to8[(cv >> 3) & 0x07];
		const u32 g  = kExpand3to8[(cv >> 6) & 0x07];
		const u32 b  = kExpand3to8[cv & 0x07];
		rgbaLUT[i]   = 0xFF000000u | (b << 16) | (g << 8) | r;
	}

	bool anyDirty = false;

	for (int tile_y = 0; tile_y < screen_size_y; tile_y++)
	{
		for (int tile_x = 0; tile_x < screen_size_x; tile_x++)
		{
			const int bat_i     = tile_x + tile_y * screen_size_x;
			const u16 bat_entry = vram[bat_i];
			const int tile_base = (bat_entry & 0x07FF) * 16;

			// Skip tiles whose BAT entry and VRAM data haven't changed since the last render.
			bool dirty = (LastRenderedFrame < 0) || (bat_entry != ShadowBAT[bat_i]);
			if (!dirty)
			{
				for (int w = 0; w < 16 && !dirty; w++)
				{
					if (pVRAMViewer->GetVRAMAccess(tile_base + w).FrameLastWritten >= LastRenderedFrame)
						dirty = true;
				}
			}
			if (!dirty)
				continue;

			ShadowBAT[bat_i] = bat_entry;
			anyDirty = true;

			const int color_tbl_base = ((bat_entry >> 12) & 0x0F) * 16;
			u8* dst_row = BackgroundBuffer + (tile_y * 8 * BufferWidth + tile_x * 8) * 4;

			for (int row = 0; row < 8; row++, dst_row += BufferWidth * 4)
			{
				const u16 word_a = vram[tile_base + row];
				const u16 word_b = vram[tile_base + row + 8];
				const u8 byte1 = (u8)(word_a & 0xFF);
				const u8 byte2 = (u8)(word_a >> 8);
				const u8 byte3 = (u8)(word_b & 0xFF);
				const u8 byte4 = (u8)(word_b >> 8);

				uint32_t* dst = (uint32_t*)dst_row;
				for (int col = 0; col < 8; col++)
				{
					const int shift = 7 - col;
					const int color = ((byte1 >> shift) & 1)        |
					                  (((byte2 >> shift) & 1) << 1) |
					                  (((byte3 >> shift) & 1) << 2) |
					                  (((byte4 >> shift) & 1) << 3);
					dst[col] = rgbaLUT[color == 0 ? 0 : color_tbl_base + color];
				}
			}
		}
	}

	LastRenderedFrame = currentFrame;
	return anyDirty;
}
