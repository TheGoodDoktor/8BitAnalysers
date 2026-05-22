#include "SpriteViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include "VRAMViewer.h"

#include <geargrafx_core.h>

#include <ImGuiSupport/ImGuiTexture.h>
#include "CodeAnalyser/UI/UIColours.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"

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
		SpriteBuffers[s] = new u8[HUC6270_MAX_SPRITE_WIDTH * HUC6270_MAX_SPRITE_HEIGHT * 4];

		for (int j = 0; j < HUC6270_MAX_SPRITE_WIDTH * HUC6270_MAX_SPRITE_HEIGHT * 4; j++)
			SpriteBuffers[s][j] = 0;
	}

	return true;
}

void FSpriteViewer::Shutdown()
{
	ResetForGame();
}

void FSpriteViewer::ClearHistory()
{
	for (auto& e : SpriteHistory)
	{
		ImGui_FreeTexture(e.Texture);
		delete[] e.PixelBuffer;
		delete[] e.VRAMSnapshot;
	}
	SpriteHistory.clear();
	HistorySelectedSprite = -1;
	FindCursor = -1;
	FindFound  = 0;
	FindTotal  = 0;
	// SlotHash is deliberately NOT cleared here so sprites don't immediately
	// re-enter history — they'll only be re-captured when their VRAM data changes.
}

void FSpriteViewer::ResetForGame()
{
	ClearHistory();
	memset(SlotHash, 0, sizeof(SlotHash));
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

	const FSpriteInfo* spriteInfo = pPCEEmu->GetVRAMViewer()->GetSpriteInfo();
	for (int i = 0; i < HUC6270_SPRITES; i++)
	{
		const int sprite_offset = i << 2;
		const u16 flags = sat[sprite_offset + 3] & 0xB98F;
		const bool x_flip = (flags & 0x0800);
		const bool y_flip = (flags & 0x8000);
		const int palette = flags & 0x0F;
		const bool mode1 = ((huc6270->GetState()->R[HUC6270_REG_MWR] >> 2) & 0x03) == 1;
		const int mode1_offset = mode1 ? (sat[sprite_offset + 2] & 1) << 5 : 0;

		const int height = spriteInfo[i].Height;
		const int width = spriteInfo[i].Width;
		const uint16_t sprite_address = spriteInfo[i].Address;

		for (int y = 0; y < height; y++)
		{
			const int flipped_y = y_flip ? (height - 1 - y) : y;
			const int tile_y = flipped_y >> 4;
			const int tile_line_offset = tile_y * 2 * 64;
			const int offset_y = flipped_y & 0xF;
			const u16 line_start = sprite_address + tile_line_offset + offset_y;

			for (int x = 0; x < width; x++)
			{
				const int flipped_x = x_flip ? (width - 1 - x) : x;
				const int tile_x = flipped_x >> 4;
				const int tile_x_offset = tile_x * 64;
				const int line = line_start + tile_x_offset + mode1_offset;

				// sam. was crashing here going out of bounds 
				if ((line + 48) < HUC6270_VRAM_SIZE)
				{
					const u16 plane1 = vram[line + 0];
					const u16 plane2 = vram[line + 16];
					const u16 plane3 = mode1 ? 0 : vram[line + 32];
					const u16 plane4 = mode1 ? 0 : vram[line + 48];

					const int pixel_x = 15 - (flipped_x & 0xF);
					u16 pixel = ((plane1 >> pixel_x) & 0x01) | (((plane2 >> pixel_x) & 0x01) << 1) | (((plane3 >> pixel_x) & 0x01) << 2) | (((plane4 >> pixel_x) & 0x01) << 3);
					pixel |= (palette << 4);
					pixel |= 0x100;

					const int color = color_table[pixel & 0x1FF];
					u8 green = ((color >> 6) & 0x07) * 255 / 7;
					u8 red = ((color >> 3) & 0x07) * 255 / 7;
					u8 blue = (color & 0x07) * 255 / 7;

					if (!(pixel & 0x0F))
					{
						if (BackgroundColour == 0)
						{
							// Grey
							red = 128;
							green = 128;
							blue = 128;
						}
						else if (BackgroundColour == 1)
						{
							// Black
							red = 0;
							green = 0;
							blue = 0;
						}
						else
						{
							// Magenta
							red = 255;
							green = 0;
							blue = 255;
						}
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
}

static uint32_t HashSpriteVRAM(const u16* pVRAM, uint16_t vramAddr, int sizeWords)
{
	uint32_t h = 2166136261u;
	const uint8_t* p = reinterpret_cast<const uint8_t*>(pVRAM + vramAddr);
	const int bytes = sizeWords * 2;
	for (int i = 0; i < bytes; i++)
		h = (h ^ p[i]) * 16777619u;
	return h;
}

void FSpriteViewer::UpdateSpriteHistory()
{
	if ((int)SpriteHistory.size() >= kMaxHistoryEntries)
		return;

	const u16* pVRAM = pPCEEmu->GetCore()->GetHuC6270_1()->GetVRAM();
	const FSpriteInfo* spriteInfo = pPCEEmu->GetVRAMViewer()->GetSpriteInfo();

	for (int s = 0; s < kNumSprites && (int)SpriteHistory.size() < kMaxHistoryEntries; s++)
	{
		const FSpriteInfo& info = spriteInfo[s];
		if (info.Width == 0 || info.Height == 0)
			continue;

		// Skip slots whose VRAM pattern area has never been written by game code
		if (pPCEEmu->GetVRAMViewer()->GetVRAMAccess(info.Address).FrameLastWritten == -1)
			continue;

		const int sizeWords = info.SizeInBytes / 2;
		if (info.Address + sizeWords > HUC6270_VRAM_SIZE)
			continue;

		const uint32_t hash = HashSpriteVRAM(pVRAM, info.Address, sizeWords);
		if (hash == SlotHash[s])
			continue;
		SlotHash[s] = hash;

		bool bAlreadySeen = false;
		for (const auto& e : SpriteHistory)
		{
			if (e.DataHash == hash && e.Width == info.Width && e.Height == info.Height)
			{
				bAlreadySeen = true;
				break;
			}
		}
		if (bAlreadySeen)
			continue;

		FHistorySpriteEntry entry;
		entry.Width       = info.Width;
		entry.Height      = info.Height;
		entry.Palette     = info.Palette;
		entry.VRAMAddress = info.Address;
		entry.DataHash    = hash;

		const int bufSize = info.Width * info.Height * 4;
		entry.PixelBuffer = new uint8_t[bufSize];
		memcpy(entry.PixelBuffer, SpriteBuffers[s], bufSize);
		entry.Texture = ImGui_CreateTextureRGBA(entry.PixelBuffer, info.Width, info.Height);

		entry.VRAMSnapshotSize = sizeWords * 2;
		entry.VRAMSnapshot = new uint8_t[entry.VRAMSnapshotSize];
		memcpy(entry.VRAMSnapshot, reinterpret_cast<const uint8_t*>(pVRAM + info.Address), entry.VRAMSnapshotSize);

		SpriteHistory.push_back(entry);
	}
}

void FSpriteViewer::Tick()
{
	ResetScreenTexture();
	UpdateSpriteBuffers();

	const FSpriteInfo* spriteInfo = pPCEEmu->GetVRAMViewer()->GetSpriteInfo();
	for (int s = 0; s < kNumSprites; s++)
		ImGui_UpdateTextureSubImageRGBA(SpriteTextures[s], SpriteBuffers[s], spriteInfo[s].Width, spriteInfo[s].Height);

	UpdateSpriteHistory();

	// Find All — process a batch of entries per frame (search only, no formatting)
	if (FindCursor >= 0 && FindCursor < (int)SpriteHistory.size())
	{
		const int kBatchSize = 8;
		const int end = FindCursor + kBatchSize < (int)SpriteHistory.size()
		              ? FindCursor + kBatchSize
		              : (int)SpriteHistory.size();
		FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

		for (int i = FindCursor; i < end; i++)
		{
			FHistorySpriteEntry& e = SpriteHistory[i];
			if (e.FoundDataAddr.IsValid())
			{
				FindFound++;
				continue;
			}

			if (e.VRAMSnapshot && e.VRAMSnapshotSize >= 64)
			{
				std::vector<FAddressRef> results = state.FindAllMemoryPatterns(e.VRAMSnapshot, 64, true, false);
				if (!results.empty())
				{
					e.FoundDataAddr = results[0];
					FindFound++;
				}
			}
		}
		FindCursor = end;
	}
}

void FSpriteViewer::FormatEntry(FHistorySpriteEntry& e)
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FDataFormattingOptions options;
	options.DataType     = EDataType::Bitmap;
	options.DisplayType  = EDataItemDisplayType::Sprite4Bpp_PCE;
	options.StartAddress = e.FoundDataAddr;
	options.ItemSize     = e.Width / 2;
	options.NoItems      = e.Height;
	options.PaletteNo    = pPCEEmu->CreateUserPalette(16 + e.Palette);
	FormatData(state, options);
	state.SetCodeAnalysisDirty(options.StartAddress);
	e.bFormatted = true;
}

static const ImVec4 cyan   = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
static const ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

void FSpriteViewer::DrawUI()
{
	HighlightSprite = -1;

	if (ImGui::BeginTabBar("##sprite_tabs"))
	{
		if (ImGui::BeginTabItem("Current"))
		{
			GeargrafxCore* core = pPCEEmu->GetCore();
			HuC6270* huc6270 = core->GetHuC6270_1();
			const u16* sat = huc6270->GetSAT();

			const FGlobalConfig* pConfig = pPCEEmu->GetGlobalConfig();
			const float scale = (float)pConfig->ImageScale;

			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();

			const FSpriteInfo* spriteInfo = pPCEEmu->GetVRAMViewer()->GetSpriteInfo();

			const float colWidth      = (float)HUC6270_MAX_SPRITE_WIDTH * scale;
			const float colHeight     = (float)HUC6270_MAX_SPRITE_HEIGHT * scale;
			const float cellMinHeight = colHeight + style.CellPadding.y * 2.0f;

			const float leftPanelWidth = 8.0f * (colWidth + 2.0f * style.CellPadding.x) + 9.0f + style.WindowPadding.x * 2.0f;

			ImVec2 p[64];
			bool grid_hovered = false;

			if (ImGui::BeginChild("##SpriteGrid", ImVec2(leftPanelWidth, 0.0f), false))
			{
				grid_hovered = ImGui::IsWindowHovered();

				ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;

				if (ImGui::BeginTable("spritetable", 8, flags))
				{
					for (int col = 0; col < 8; col++)
						ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, colWidth);

					for (int s = 0; s < HUC6270_SPRITES; s++)
					{
						if (s % 8 == 0)
							ImGui::TableNextRow(ImGuiTableRowFlags_None, cellMinHeight);

						ImGui::TableNextColumn();

						u16 sprite_flags = sat[(s * 4) + 3] & 0xB98F;
						float fwidth  = k_huc6270_sprite_width[(sprite_flags >> 8) & 0x01] * scale;
						float fheight = k_huc6270_sprite_height[(sprite_flags >> 12) & 0x03] * scale;
						float tex_h = fwidth / 32.0f / scale;
						float tex_v = fheight / 64.0f / scale;

						if (fwidth < colWidth)
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (colWidth - fwidth) * 0.5f);
						if (fheight < colHeight)
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (colHeight - fheight) * 0.5f);

						p[s] = ImGui::GetCursorScreenPos();

						ImGui::Image(SpriteTextures[s], ImVec2(fwidth, fheight), ImVec2(0.0f, 0.0f), ImVec2(tex_h, tex_v));

						if (ImGui::IsItemClicked(0))
							SelectedSprite = s;

						if (ImGui::IsItemHovered() && bShowMagnifier)
						{
							ImGui::BeginTooltip();
							const float magAmount = 4.0f;
							ImGui::Image(SpriteTextures[s], ImVec2(fwidth * magAmount, fheight * magAmount), ImVec2(0.0f, 0.0f), ImVec2(tex_h, tex_v));
							ImGui::EndTooltip();
						}

						float mouse_x = io.MousePos.x - p[s].x;
						float mouse_y = io.MousePos.y - p[s].y;
						const bool bHovered   = grid_hovered && (mouse_x >= 0.0f) && (mouse_x < fwidth) && (mouse_y >= 0.0f) && (mouse_y < fheight);
						const bool bHighlight = pPCEEmu->GetVRAMViewer()->GetSpriteHighlight() == s;
						const bool bSelected  = (SelectedSprite == s);

						if (bHighlight || bSelected || bHovered)
						{
							ImDrawList* draw_list = ImGui::GetWindowDrawList();
							ImColor rectColor = bHighlight ? ImColor(Colours::GetFlashColour()) :
							                    bSelected  ? ImColor(yellow) :
							                                 ImColor(cyan);
							draw_list->AddRect(ImVec2(p[s].x, p[s].y), ImVec2(p[s].x + fwidth, p[s].y + fheight), rectColor, 0.f, 0, 2.f);
						}
					}
					ImGui::EndTable();
				}

				const char* colours[] = { "Grey", "Black", "Magenta" };
				ImGui::Combo("Background", &BackgroundColour, colours, IM_ARRAYSIZE(colours));
				ImGui::Checkbox("Show Magnifier", &bShowMagnifier);
			}
			ImGui::EndChild();

			ImGui::SameLine();

			if (ImGui::BeginChild("##SpriteDetails", ImVec2(0.0f, 0.0f), true))
			{
				if (SelectedSprite >= 0)
					DrawSpriteDetails(SelectedSprite);
				else
					ImGui::TextDisabled("Click a sprite to see details");
			}
			ImGui::EndChild();

			// Update HighlightSprite for cross-viewer communication (e.g. VRAMViewer map)
			for (int s = 0; s < HUC6270_SPRITES; s++)
			{
				float mouse_x = io.MousePos.x - p[s].x;
				float mouse_y = io.MousePos.y - p[s].y;
				u16 sprite_flags = sat[(s * 4) + 3] & 0xB98F;
				float fwidth  = k_huc6270_sprite_width[(sprite_flags >> 8) & 0x01] * scale;
				float fheight = k_huc6270_sprite_height[(sprite_flags >> 12) & 0x03] * scale;

				if (grid_hovered && (mouse_x >= 0.0f) && (mouse_x < fwidth) && (mouse_y >= 0.0f) && (mouse_y < fheight))
				{
					HighlightSprite = s;
					break;
				}
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("History"))
		{
			DrawHistoryTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Results"))
		{
			DrawResultsTab();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void FSpriteViewer::DrawHistoryTab()
{
	const FGlobalConfig* pConfig = pPCEEmu->GetGlobalConfig();
	const float scale = (float)pConfig->ImageScale;

	// Toolbar
	ImGui::Text("%d sprites captured", (int)SpriteHistory.size());
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
		ClearHistory();
	ImGui::SameLine();

	if (FindCursor < 0)
	{
		if (!SpriteHistory.empty() && ImGui::Button("Find All"))
		{
			FindCursor = 0;
			FindFound  = 0;
			FindTotal  = (int)SpriteHistory.size();
		}
	}
	else if (FindCursor < FindTotal)
	{
		ImGui::Text("Searching... %d/%d  (%d found)", FindCursor, FindTotal, FindFound);
	}
	else
	{
		ImGui::Text("Done: %d / %d found", FindFound, FindTotal);
		ImGui::SameLine();
		if (ImGui::Button("OK"))
			FindCursor = -1;
	}

	ImGui::Separator();

	const float colWidth  = (float)HUC6270_MAX_SPRITE_WIDTH  * scale;
	const float colHeight = (float)HUC6270_MAX_SPRITE_HEIGHT * scale;
	const float leftPanelWidth = 8.0f * (colWidth + 2.0f * ImGui::GetStyle().CellPadding.x)
	                           + 9.0f + ImGui::GetStyle().WindowPadding.x * 2.0f;

	if (ImGui::BeginChild("##HistoryGrid", ImVec2(leftPanelWidth, 0.0f), false))
	{
		if (!SpriteHistory.empty())
		{
			ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX;
			if (ImGui::BeginTable("historytable", 8, flags))
			{
				for (int col = 0; col < 8; col++)
					ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, colWidth);

				for (int i = 0; i < (int)SpriteHistory.size(); i++)
				{
					if (i % 8 == 0)
						ImGui::TableNextRow();
					ImGui::TableNextColumn();

					const FHistorySpriteEntry& e = SpriteHistory[i];
					const float fw = (float)e.Width  * scale;
					const float fh = (float)e.Height * scale;

					if (fw < colWidth)
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (colWidth - fw) * 0.5f);
					if (fh < colHeight)
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (colHeight - fh) * 0.5f);

					ImGui::Image(e.Texture, ImVec2(fw, fh), ImVec2(0, 0), ImVec2(1.0f, 1.0f));

					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Image(e.Texture, ImVec2(fw * 4.0f, fh * 4.0f), ImVec2(0, 0), ImVec2(1.0f, 1.0f));
						ImGui::EndTooltip();
					}

					if (ImGui::IsItemClicked(0))
						HistorySelectedSprite = i;

					if (HistorySelectedSprite == i)
					{
						const ImVec2 rmin = ImGui::GetItemRectMin();
						const ImVec2 rmax = ImGui::GetItemRectMax();
						ImGui::GetWindowDrawList()->AddRect(rmin, rmax, IM_COL32(255, 255, 0, 255), 0.f, 0, 2.f);
					}
					else if (e.bFormatted)
					{
						const ImVec2 rmin = ImGui::GetItemRectMin();
						const ImVec2 rmax = ImGui::GetItemRectMax();
						ImGui::GetWindowDrawList()->AddRect(rmin, rmax, IM_COL32(0, 200, 0, 255), 0.f, 0, 1.f);
					}
				}
				ImGui::EndTable();
			}
		}
		else
		{
			ImGui::TextDisabled("No sprites captured yet");
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	if (ImGui::BeginChild("##HistoryDetail", ImVec2(0.0f, 0.0f), true))
	{
		if (HistorySelectedSprite >= 0 && HistorySelectedSprite < (int)SpriteHistory.size())
			DrawHistoryDetails(HistorySelectedSprite);
		else
			ImGui::TextDisabled("Click a sprite to see details");
	}
	ImGui::EndChild();
}

void FSpriteViewer::DrawResultsTab()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	int foundCount = 0;
	for (const FHistorySpriteEntry& e : SpriteHistory)
		if (e.FoundDataAddr.IsValid()) foundCount++;

	ImGui::Text("%d result(s)", foundCount);

	if (foundCount > 0)
	{
		ImGui::SameLine();
		if (ImGui::Button("Format All Found"))
		{
			for (FHistorySpriteEntry& e : SpriteHistory)
			{
				if (e.FoundDataAddr.IsValid() && !e.bFormatted)
					FormatEntry(e);
			}
		}
	}

	ImGui::Separator();

	if (foundCount == 0)
	{
		ImGui::TextDisabled("Run 'Find All' in the History tab to populate results");
		return;
	}

	ImGuiTableFlags tblFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg
	                         | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY;
	if (ImGui::BeginTable("##resultstable", 5, tblFlags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("",         ImGuiTableColumnFlags_WidthFixed,   36.0f);
		ImGui::TableSetupColumn("Size",     ImGuiTableColumnFlags_WidthFixed,   52.0f);
		ImGui::TableSetupColumn("Address",  ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Status",   ImGuiTableColumnFlags_WidthFixed,   68.0f);
		ImGui::TableSetupColumn("",         ImGuiTableColumnFlags_WidthFixed,   52.0f);
		ImGui::TableHeadersRow();

		for (int i = 0; i < (int)SpriteHistory.size(); i++)
		{
			FHistorySpriteEntry& e = SpriteHistory[i];
			if (!e.FoundDataAddr.IsValid())
				continue;

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			const float tw = (float)e.Width  * 0.5f;
			const float th = (float)e.Height * 0.5f;
			ImGui::Image(e.Texture, ImVec2(tw, th), ImVec2(0, 0), ImVec2(1.0f, 1.0f));

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%dx%d", e.Width, e.Height);

			ImGui::TableSetColumnIndex(2);
			DrawAddressLabel(state, viewState, e.FoundDataAddr);

			ImGui::TableSetColumnIndex(3);
			if (e.bFormatted)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 200, 0, 255));
				ImGui::TextUnformatted("Formatted");
				ImGui::PopStyleColor();
			}

			ImGui::TableSetColumnIndex(4);
			if (!e.bFormatted)
			{
				ImGui::PushID(i);
				if (ImGui::SmallButton("Format"))
					FormatEntry(e);
				ImGui::PopID();
			}
		}
		ImGui::EndTable();
	}
}

void FSpriteViewer::DrawHistoryDetails(int index)
{
	FHistorySpriteEntry& e = SpriteHistory[index];
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	ImGui::Text("History entry %d", index);
	ImGui::Separator();

	ImGuiTableFlags tblFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV;
	if (ImGui::BeginTable("##histdetails", 2, tblFlags))
	{
		ImGui::TableSetupColumn("Attribute", ImGuiTableColumnFlags_WidthFixed, 90.0f);
		ImGui::TableSetupColumn("Value",     ImGuiTableColumnFlags_WidthStretch);

		auto Row = [](const char* attr, const char* val)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(attr);
			ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(val);
		};

		char buf[32];
		snprintf(buf, sizeof(buf), "$%04X", e.VRAMAddress); // this is a 16 bit address.
		Row("VRAM Addr", buf); 
		snprintf(buf, sizeof(buf), "%d", e.Width);
		Row("Width", buf);
		snprintf(buf, sizeof(buf), "%d", e.Height);
		Row("Height", buf);
		snprintf(buf, sizeof(buf), "%d", e.Palette);
		Row("Palette", buf);

		ImGui::EndTable();
	}

	ImGui::Spacing();

	if (e.FoundDataAddr.IsValid())
	{
		DrawAddressLabel(state, viewState, e.FoundDataAddr);
		ImGui::Spacing();

		if (ImGui::Button("Format as Sprite"))
			FormatEntry(e);
	}
	else if (e.VRAMSnapshot && ImGui::Button("Find in Memory"))
	{
		std::vector<FAddressRef> results = state.FindAllMemoryPatterns(e.VRAMSnapshot, e.VRAMSnapshotSize, true, false);
		if (!results.empty())
			e.FoundDataAddr = results[0];
	}
}

int FSpriteViewer::CountSpritesFoundInMemory(int& outHistorySize)
{
	outHistorySize = (int)SpriteHistory.size();
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	int found = 0;
	for (FHistorySpriteEntry& e : SpriteHistory)
	{
		if (e.FoundDataAddr.IsValid())
		{
			found++;
			continue;
		}
		if (e.VRAMSnapshot && e.VRAMSnapshotSize > 0)
		{
			std::vector<FAddressRef> results = state.FindAllMemoryPatterns(e.VRAMSnapshot, e.VRAMSnapshotSize, true, false);
			if (!results.empty())
			{
				e.FoundDataAddr = results[0];
				found++;
			}
		}
	}
	return found;
}

void FSpriteViewer::DrawSpriteDetails(int spriteIndex)
{
	HuC6270* huc6270 = pPCEEmu->GetCore()->GetHuC6270_1();
	const u16* sat = huc6270->GetSAT();
	FVRAMViewer* pVRAMViewer = pPCEEmu->GetVRAMViewer();
	const FSpriteInfo& info = pVRAMViewer->GetSpriteInfo()[spriteIndex];
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	const int offset   = spriteIndex * 4;
	const u16 yWord    = sat[offset + 0];
	const u16 xWord    = sat[offset + 1];
	const u16 patWord  = sat[offset + 2];
	const u16 flgWord  = sat[offset + 3] & 0xB98F;

	const u16  pattern  = (patWord >> 1) & 0x03FF;
	const bool h_flip   = (flgWord & 0x0800) != 0;
	const bool v_flip   = (flgWord & 0x8000) != 0;
	const int  palette  = flgWord & 0x0F;
	const bool priority = (flgWord & 0x0080) != 0;

	const uint16_t satBase = huc6270->GetState()->R[HUC6270_REG_DVSSR];
	const uint16_t yAddr   = satBase + spriteIndex * 4 + 0;
	const uint16_t xAddr   = satBase + spriteIndex * 4 + 1;
	const uint16_t patAddr = satBase + spriteIndex * 4 + 2;
	const uint16_t flgAddr = satBase + spriteIndex * 4 + 3;

	const FAddressRef yWriter   = pVRAMViewer->GetVRAMAccess(yAddr).LastWriter;
	const FAddressRef xWriter   = pVRAMViewer->GetVRAMAccess(xAddr).LastWriter;
	const FAddressRef patWriter = pVRAMViewer->GetVRAMAccess(patAddr).LastWriter;
	const FAddressRef flgWriter = pVRAMViewer->GetVRAMAccess(flgAddr).LastWriter;

	ImGui::Text("Sprite %d", spriteIndex);
	ImGui::Separator();

	ImGuiTableFlags tblFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV;
	if (ImGui::BeginTable("##details", 3, tblFlags))
	{
		ImGui::TableSetupColumn("Attribute",   ImGuiTableColumnFlags_WidthFixed,   90.0f);
		ImGui::TableSetupColumn("Value",       ImGuiTableColumnFlags_WidthFixed,  120.0f);
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

		char buf[64];

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("SAT Entry");
		ImGui::TableSetColumnIndex(1); ImGui::Text("%d", spriteIndex);
		ImGui::TableSetColumnIndex(2); ImGui::TextDisabled("--");

		snprintf(buf, sizeof(buf), "$%03X  (%d)", info.XPos, info.XPos);
		Row("X Position", buf, xWriter);

		snprintf(buf, sizeof(buf), "$%03X  (%d)", info.YPos, info.YPos);
		Row("Y Position", buf, yWriter);

		snprintf(buf, sizeof(buf), "$%04X", info.Address);
		Row("VRAM Address", buf, patWriter);

		snprintf(buf, sizeof(buf), "%d", info.Width);
		Row("Width", buf, flgWriter);

		snprintf(buf, sizeof(buf), "%d", info.Height);
		Row("Height", buf, flgWriter);

		snprintf(buf, sizeof(buf), "$%03X  (%d)", pattern, pattern);
		Row("Pattern", buf, patWriter);

		snprintf(buf, sizeof(buf), "%d", palette);
		Row("Palette", buf, flgWriter);

		Row("H Flip",   h_flip   ? "YES" : "NO", flgWriter);
		Row("V Flip",   v_flip   ? "YES" : "NO", flgWriter);
		Row("Priority", priority ? "YES" : "NO", flgWriter);

		ImGui::EndTable();
	}

	ImGui::Spacing();
	ImGui::TextDisabled("SAT VRAM base: $%04X", satBase);
	ImGui::TextDisabled("Raw: Y=$%04X  X=$%04X  Pat=$%04X  Flg=$%04X", yWord, xWord, patWord, flgWord);

	ImGui::Spacing();
	ImGui::Separator();

	if (spriteIndex != LastSearchedSprite)
	{
		bFoundSpriteData = false;
		LastSearchedSprite = spriteIndex;
	}

	if (ImGui::Button("Find in Memory"))
	{
		bFoundSpriteData = false;

		const u16* pVRAM = huc6270->GetVRAM();
		const uint16_t vramAddr = info.Address;

		if (vramAddr < HUC6270_VRAM_SIZE && (vramAddr + 32) <= HUC6270_VRAM_SIZE)
		{
			const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(pVRAM + vramAddr);
			std::vector<FAddressRef> results = state.FindAllMemoryPatterns(pBytes, 64, true, false);
			if (!results.empty())
			{
				FoundSpriteDataAddr = results[0];
				bFoundSpriteData = true;
			}
		}
	}

	if (bFoundSpriteData)
	{
		ImGui::SameLine();
		DrawAddressLabel(state, viewState, FoundSpriteDataAddr);

		if (ImGui::Button("Format as Sprite"))
		{
			FDataFormattingOptions options;
			options.DataType    = EDataType::Bitmap;
			options.DisplayType = EDataItemDisplayType::Sprite4Bpp_PCE;
			options.StartAddress = FoundSpriteDataAddr;
			options.ItemSize    = info.Width / 2;
			options.NoItems     = info.Height;
			options.PaletteNo   = pPCEEmu->CreateUserPalette(16 + info.Palette);
			FormatData(state, options);
			state.SetCodeAnalysisDirty(options.StartAddress);
		}
	}
}
