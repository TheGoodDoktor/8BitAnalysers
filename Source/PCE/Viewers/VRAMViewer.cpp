#include "VRAMViewer.h"
#include "SpriteViewer.h"

#include <string.h>
#include <imgui.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "CodeAnalyser/UI/UIColours.h"
#include "optick/optick.h"
#include "../PCEEmu.h"

#include "Misc/EmuBase.h"

static const int kMemoryViewImageWidth = 128;
static const int kMemoryViewImageHeight = 256;

FVRAMViewer::FVRAMViewer(FEmuBase* pEmu)
	: FViewerBase(pEmu)
{
	Name = "VRAM";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FVRAMViewer::Init(void)
{
	MemoryViewImage = new FGraphicsView(kMemoryViewImageWidth, kMemoryViewImageHeight);
	MemoryViewImage->Clear(0xff000000);	

	return true;
}

void FVRAMViewer::Reset(void)
{
	for (FVRAMAccess& access : Access)
	{
		access.FrameLastWritten = -1;
		access.LastWriter = FAddressRef::Invalid();
	}
	memset(SpriteIndexLookup, -1, sizeof(SpriteIndexLookup));
	memset(BGTileLookup,      -1, sizeof(BGTileLookup));
	memset(BATLookup,         -1, sizeof(BATLookup));
	memset(SATWordLookup,     -1, sizeof(SATWordLookup));
}

void FVRAMViewer::DrawUI(void)
{
	DrawPhysicalMemoryOverview();
}

void	FVRAMViewer::DrawLegend()
{
	auto LegendRow = [](const char* id, uint32_t col, const char* label)
	{
		ImGui::ColorButton(id, ImGui::ColorConvertU32ToFloat4(col), ImGuiColorEditFlags_NoTooltip);
		ImGui::SameLine();
		ImGui::Text("%s", label);
	};

	ImGui::BeginTooltip();

	LegendRow("Unwritten",        kUnwrittenCol,         "Unwritten");
	LegendRow("Write",            kUnknownWriteCol,      "Unknown write");
	LegendRow("ActiveWrite",      kUnknownWriteActiveCol,"Unknown write (active)");
	LegendRow("BATWrite",         kBATWriteCol,          "BAT (background attribute table)");
	LegendRow("BATWriteActive",   kBATWriteActiveCol,    "BAT (active write)");
	LegendRow("BGTileWrite",      kBGTileWriteCol,       "BG tile data");
	LegendRow("BGTileWriteActive",kBGTileWriteActiveCol, "BG tile data (active write)");
	LegendRow("SpriteWrite",      kSpriteWriteCol,       "Sprite pattern data");
	LegendRow("SpriteWriteActive",kSpriteWriteActiveCol, "Sprite pattern data (active write)");
	LegendRow("SATWrite",         kSATWriteCol,          "SAT (sprite attribute table)");
	LegendRow("SATWriteActive",   kSATWriteActiveCol,    "SAT (active write)");

	ImGui::EndTooltip();
}

void	FVRAMViewer::DrawPhysicalMemoryOverview()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	DrawUtilisationMap(state,pPix);

	FGlobalConfig* pConfig = state.pGlobalConfig;

	ImGui::InputInt("Scale", &pConfig->VRAMViewerScale, 1, 1);
	pConfig->VRAMViewerScale = MAX(1, pConfig->VRAMViewerScale);	// clamp
	
	const float scale = ImGui_GetScaling() * (float)pConfig->VRAMViewerScale;

	MemoryViewImage->UpdateTexture();

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 pos = ImGui::GetCursorScreenPos();

	// Draw Image
	const int height = kMemoryViewImageHeight;
	const ImVec2 size((float)kMemoryViewImageWidth * scale, height * scale);
	const ImVec2 uv0(0, 0);
	const ImVec2 uv1(1.0f, 1.0f);
	ImGui::Image((void*)MemoryViewImage->GetTexture(), size,uv0,uv1);

	const bool bMapIsHovered = ImGui::IsItemHovered();
	SpriteHighlight = -1;

	ImGui::SameLine();
	ImGui::Button("?");

	if (ImGui::IsItemHovered())
	{
		DrawLegend();
	}

	ImDrawList* dl = ImGui::GetWindowDrawList();

	if (bMapIsHovered)
	{
		FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

		const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = (int)((io.MousePos.y - pos.y) / scale);
		const uint16_t addr = (uint16_t)(xp + yp * kMemoryViewImageWidth);

		ImGui::Text("VRAM: $%04X", addr);

		GeargrafxCore*  pCore      = pPCEEmu->GetCore();
		HuC6270*        huc6270    = pCore->GetHuC6270_1();
		HuC6260*        huc6260    = pCore->GetHuC6260();
		const u16*      vram       = huc6270->GetVRAM();
		const u16*      colorTable = huc6260->GetColorTable();
		const u16*      sat        = huc6270->GetSAT();
		FSpriteViewer*  pSpriteViewer = pPCEEmu->GetSpriteViewer();

		const FAddressRef& lastWriter  = Access[addr].LastWriter;
		const int spriteIndex          = GetSpriteIndexForAddress(addr);
		const int satWord              = GetSATWordForAddress(addr);
		const int bgTileIndex          = GetBGTileIndexForAddress(addr);
		const int batIndex             = GetBATIndexForAddress(addr);

		// Helper: draw an 8x8 BG tile inline using ImDrawList (no texture needed).
		// Each tile pixel is rendered as a kPixSz x kPixSz filled rectangle.
		auto DrawTilePreview = [&](int tileIdx, int palette)
		{
			if (tileIdx < 0 || tileIdx * 16 + 15 >= HUC6270_VRAM_SIZE)
				return;
			static const u8 kE38[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
			const float kPixSz = 12.0f;
			const ImVec2 base  = ImGui::GetCursorScreenPos();
			ImDrawList*  dl    = ImGui::GetWindowDrawList();
			const int tile_base = tileIdx * 16;
			for (int row = 0; row < 8; row++)
			{
				const u16 wa = vram[tile_base + row];
				const u16 wb = vram[tile_base + row + 8];
				const u8  b1 = (u8)(wa & 0xFF), b2 = (u8)(wa >> 8);
				const u8  b3 = (u8)(wb & 0xFF), b4 = (u8)(wb >> 8);
				for (int col = 0; col < 8; col++)
				{
					const int sh    = 7 - col;
					const int color = ((b1 >> sh) & 1) | (((b2 >> sh) & 1) << 1) |
					                  (((b3 >> sh) & 1) << 2) | (((b4 >> sh) & 1) << 3);
					const u16 cv = colorTable[color == 0 ? 0 : palette * 16 + color];
					const u8 r = kE38[(cv >> 3) & 7];
					const u8 g = kE38[(cv >> 6) & 7];
					const u8 b = kE38[cv & 7];
					ImVec2 tl(base.x + col * kPixSz, base.y + row * kPixSz);
					ImVec2 br(tl.x + kPixSz, tl.y + kPixSz);
					dl->AddRectFilled(tl, br, IM_COL32(r, g, b, 255));
				}
			}
			ImGui::Dummy(ImVec2(8 * kPixSz, 8 * kPixSz));
		};

		if (satWord != -1)
		{
			// SAT word — show which sprite and which attribute field
			const int  spriteIdx = satWord >> 2;
			const int  field     = satWord & 3;
			static const char* kFieldName[] = { "Y Position", "X Position", "Pattern", "Flags" };
			ImGui::BeginTooltip();
			ImGui::Text("SAT — Sprite %d: %s", spriteIdx, kFieldName[field]);
			ImGui::Text("Value: $%04X", sat[satWord]);
			if (lastWriter.IsValid()) { ImGui::Text("Last writer:"); ImGui::SameLine(); DrawAddressLabel(state, viewState, lastWriter); }
			ImGui::EndTooltip();
			SpriteHighlight = spriteIdx;
		}
		else if (spriteIndex != -1)
		{
			// Sprite pattern data
			ImGui::BeginTooltip();
			ImGui::Text("Sprite %d pattern data", spriteIndex);
			const FSpriteInfo& info = SpriteInfo[spriteIndex];
			if (pSpriteViewer && info.Width > 0 && info.Height > 0)
			{
				const float uv_w = (float)info.Width  / HUC6270_MAX_SPRITE_WIDTH;
				const float uv_h = (float)info.Height / HUC6270_MAX_SPRITE_HEIGHT;
				ImGui::Image(pSpriteViewer->GetSpriteTexture(spriteIndex),
				             ImVec2((float)info.Width * 4.0f, (float)info.Height * 4.0f),
				             ImVec2(0, 0), ImVec2(uv_w, uv_h));
			}
			if (lastWriter.IsValid()) { ImGui::Text("Last writer:"); ImGui::SameLine(); DrawAddressLabel(state, viewState, lastWriter); }
			ImGui::EndTooltip();
			SpriteHighlight = spriteIndex;
		}
		else if (batIndex != -1)
		{
			// BAT entry — show tile and draw preview
			const u16 batEntry  = vram[batIndex];
			const int tileIndex = batEntry & 0x07FF;
			const int palette   = (batEntry >> 12) & 0x0F;
			ImGui::BeginTooltip();
			ImGui::Text("BAT entry %d", batIndex);
			DrawTilePreview(tileIndex, palette);
			ImGui::Text("Tile $%03X  Palette %d  VRAM $%04X", tileIndex, palette, tileIndex * 16);
			if (lastWriter.IsValid()) { ImGui::Text("Last writer:"); ImGui::SameLine(); DrawAddressLabel(state, viewState, lastWriter); }
			ImGui::EndTooltip();
		}
		else if (bgTileIndex != -1)
		{
			// BG tile character data — find palette from first BAT entry that uses it
			const int screen_reg = (huc6270->GetState()->R[HUC6270_REG_MWR] >> 4) & 0x07;
			const int bat_size   = k_huc6270_screen_size_x[screen_reg] * k_huc6270_screen_size_y[screen_reg];
			int palette = 0;
			for (int i = 0; i < bat_size && i < HUC6270_VRAM_SIZE; i++)
			{
				if ((vram[i] & 0x07FF) == (u16)bgTileIndex) { palette = (vram[i] >> 12) & 0x0F; break; }
			}
			ImGui::BeginTooltip();
			ImGui::Text("BG tile $%03X (VRAM $%04X)", bgTileIndex, bgTileIndex * 16);
			DrawTilePreview(bgTileIndex, palette);
			ImGui::Text("Palette %d  (first BAT reference)", palette);
			if (lastWriter.IsValid()) { ImGui::Text("Last writer:"); ImGui::SameLine(); DrawAddressLabel(state, viewState, lastWriter); }
			ImGui::EndTooltip();
		}
		else if (Access[addr].FrameLastWritten != -1)
		{
			// Unknown written region
			ImGui::BeginTooltip();
			ImGui::Text("VRAM $%04X  (unclassified write)", addr);
			if (lastWriter.IsValid()) { ImGui::Text("Last writer:"); ImGui::SameLine(); DrawAddressLabel(state, viewState, lastWriter); }
			ImGui::EndTooltip();
		}

		if (ImGui::IsMouseDoubleClicked(0) && lastWriter.IsValid())
			viewState.GoToAddress(lastWriter, false);
	}
	else
	{
		ImGui::NewLine();
	}
}

void FVRAMViewer::DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix)
{
	const int frameThreshold = 4;
	const int currentFrameNo = state.CurrentFrameNo;

	for (uint16_t addr = 0; addr < HUC6270_VRAM_SIZE; addr++)
	{
		const bool written     = Access[addr].FrameLastWritten != -1;
		const bool activeWrite = written && (currentFrameNo - Access[addr].FrameLastWritten < frameThreshold);

		const int  spriteIndex = GetSpriteIndexForAddress(addr);
		const bool bIsSAT      = SATWordLookup[addr] != -1;
		const bool bIsSprite   = spriteIndex != -1;
		const bool bIsBGTile   = BGTileLookup[addr] != -1;
		const bool bIsBAT      = BATLookup[addr] != -1;

		uint32_t drawCol = kUnwrittenCol;

		if (bIsSAT)
		{
			drawCol = activeWrite ? kSATWriteActiveCol : kSATWriteCol;
		}
		else if (bIsSprite)
		{
			if (SpriteHighlight == spriteIndex)
				drawCol = Colours::GetFlashColour();
			else
				drawCol = activeWrite ? kSpriteWriteActiveCol : kSpriteWriteCol;
		}
		else if (bIsBGTile)
		{
			drawCol = activeWrite ? kBGTileWriteActiveCol : kBGTileWriteCol;
		}
		else if (bIsBAT)
		{
			drawCol = activeWrite ? kBATWriteActiveCol : kBATWriteCol;
		}
		else if (written)
		{
			drawCol = activeWrite ? kUnknownWriteActiveCol : kUnknownWriteCol;
		}

		*pPix++ = drawCol;
	}
}

void FVRAMViewer::RegisterRead(uint16_t vramAddress, FAddressRef reader)
{
	if (vramAddress < HUC6270_VRAM_SIZE)
	{
		FVRAMAccess& access = Access[vramAddress];
		access.FrameLastRead = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
		access.LastReader = reader;
	}
}

void FVRAMViewer::RegisterWrite(uint16_t vramAddress, FAddressRef writer)
{
	if (vramAddress < HUC6270_VRAM_SIZE)
	{
		FVRAMAccess& access = Access[vramAddress];
		access.FrameLastWritten = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
		access.LastWriter = writer;
	}
}

// Cache sprite info, such as dimensions and vram address. 
void FVRAMViewer::Tick()
{
	GeargrafxCore* pCore = pPCEEmu->GetCore();

	HuC6270* huc6270 = pCore->GetHuC6270_1();
	u16* vram = huc6270->GetVRAM();
	u16* sat = huc6270->GetSAT();

	for (int i = 0; i < HUC6270_SPRITES; i++)
	{
		const int sprite_offset = i << 2;
		u16* spriteSat = sat + sprite_offset;

		const u16 flags = spriteSat[3] & 0xB98F;
		const int cgx = (flags >> 8) & 0x01;
		const int cgy = (flags >> 12) & 0x03;
		SpriteInfo[i].Width = k_huc6270_sprite_width[cgx];
		SpriteInfo[i].Height = k_huc6270_sprite_height[cgy];
		SpriteInfo[i].SizeInBytes = (SpriteInfo[i].Width * SpriteInfo[i].Height) >> 2;

		SpriteInfo[i].XPos = spriteSat[0];
		SpriteInfo[i].YPos = spriteSat[1];

		u16 pattern = (spriteSat[2] >> 1) & 0x3FF;
		pattern &= k_huc6270_sprite_mask_width[cgx];
		pattern &= k_huc6270_sprite_mask_height[cgy];
		SpriteInfo[i].Address = pattern << 6;
	}

	// Rebuild sprite address lookup
	memset(SpriteIndexLookup, -1, sizeof(SpriteIndexLookup));
	for (int i = 0; i < HUC6270_SPRITES; i++)
	{
		const uint16_t start = SpriteInfo[i].Address;
		const uint16_t end   = start + SpriteInfo[i].SizeInBytes;
		for (uint16_t addr = start + 1; addr < end && addr < HUC6270_VRAM_SIZE; addr++)
			SpriteIndexLookup[addr] = (int16_t)i;
	}

	// Rebuild SAT lookup (256 words at the DVSSR base address)
	memset(SATWordLookup, -1, sizeof(SATWordLookup));
	const uint16_t satBase = huc6270->GetState()->R[HUC6270_REG_DVSSR];
	for (int i = 0; i < HUC6270_SAT_SIZE; i++)
	{
		const uint16_t addr = satBase + (uint16_t)i;
		if (addr < HUC6270_VRAM_SIZE)
			SATWordLookup[addr] = (int16_t)i;
	}

	// Rebuild BAT and BG-tile lookups
	memset(BATLookup,    -1, sizeof(BATLookup));
	memset(BGTileLookup, -1, sizeof(BGTileLookup));
	const int screen_reg = (huc6270->GetState()->R[HUC6270_REG_MWR] >> 4) & 0x07;
	const int bat_w      = k_huc6270_screen_size_x[screen_reg];
	const int bat_h      = k_huc6270_screen_size_y[screen_reg];
	const int bat_size   = bat_w * bat_h;
	for (int i = 0; i < bat_size && i < HUC6270_VRAM_SIZE; i++)
	{
		BATLookup[i] = (int16_t)i;

		const int tile_index = vram[i] & 0x07FF;
		const int tile_base  = tile_index * 16;
		for (int w = 0; w < 16; w++)
		{
			const int addr = tile_base + w;
			if (addr < HUC6270_VRAM_SIZE)
				BGTileLookup[addr] = (int16_t)tile_index;
		}
	}
}

// todo: deal with the fact there can be multiple sprites sharing the same adddress.
// they can have the same address but different palettes
int FVRAMViewer::GetSpriteIndexForAddress(uint16_t addr) const
{
	return SpriteIndexLookup[addr];
}
