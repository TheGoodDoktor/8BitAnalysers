#include "PCEGraphicsViewer.h"

#include <algorithm>
#include <Util/GraphicsView.h>
#include <CodeAnalyser/CodeAnalyser.h>
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include <ImGuiSupport/ImGuiScaling.h>
#include <ImGuiSupport/ImGuiTexture.h>
#include "PCEGraphicsView.h"

#include "../PCEEmu.h"
#include "VRAMViewer.h"
#include <geargrafx_core.h>


void FPCEGraphicsViewer::OnFrameStart(const uint16_t* sat)
{
	memcpy(FrameSAT, sat, sizeof(FrameSAT));
	LastRenderedScanline = -1;
}

void FPCEGraphicsViewer::OnScanlineDraw(int rasterLine, uint16_t bxr, int32_t byrEff, uint16_t mwr, uint16_t cr)
{
	if (rasterLine >= 0 && rasterLine < kMaxScanlines)
	{
		ScanlineSnapshots[rasterLine] = { bxr, byrEff, mwr, cr };
		LastRenderedScanline = rasterLine;
	}
}

const uint32_t* FPCEGraphicsViewer::GetCurrentPalette() const
{
	// Return first BG palette as a sensible fallback
	return GetPaletteFromPaletteNo(0);
}

bool FPCEGraphicsViewer::Init()
{
	pPCEEmu = (FPCEEmu*)pEmulator;
	FGraphicsViewer::Init();
	BitmapFormat = EBitmapFormat::Sprite4Bpp_PCE;

#ifndef NDEBUG
	// test view - REMOVE
	pTestPCEGraphicsView = new FPCEGraphicsView(pPCEEmu, 256, 256);
	pTestPCEGraphicsView->Clear(0xfffff00);
#endif
	return true;
}

void FPCEGraphicsViewer::DrawUI(void)
{
	if (ImGui::BeginTabBar("GraphicsViewTabBar"))
	{
		if (ImGui::BeginTabItem("GFX"))
		{
			DrawCharacterGraphicsViewer();
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Screen"))
		{
			DrawScreenViewer();
			ImGui::EndTabItem();
		}

#ifndef NDEBUG
		if (ImGui::BeginTabItem("Test"))
		{
			DrawTest();
			ImGui::EndTabItem();
		}
#endif
	}
	ImGui::EndTabBar();
}

// Games that don't draw properly:
// Rabio Lepus. Missing stars in intro starfield. Missing SPECIAL sprites
// Magical chase. Sprites drawn in front of background status window.
// Atomic Robokid missing Special sprites
void FPCEGraphicsViewer::DrawScreenViewer()
{
	bool bRecreateTexture = false;
	static bool bDrawPass[4] = { true };
#ifndef NDEBUG
	if (ImGui::Checkbox("Pass 1", &bDrawPass[0]))
		bRecreateTexture = true;

	if (ImGui::Checkbox("Pass 2", &bDrawPass[1]))
		bRecreateTexture = true;

	if (ImGui::Checkbox("Pass 3", &bDrawPass[2]))
		bRecreateTexture = true;

	if (ImGui::Checkbox("Pass 4", &bDrawPass[3]))
		bRecreateTexture = true;
#endif

	GeargrafxCore* pCore = pPCEEmu->GetCore();
	HuC6270* huc6270 = pCore->GetHuC6270_1();
	HuC6260* huc6260 = pCore->GetHuC6260();
	HuC6270::HuC6270_State* pHWState = huc6270->GetState();
	FVRAMViewer* pVRAMViewer = pPCEEmu->GetVRAMViewer();
	FCodeAnalysisState& codeAnalysis = GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = codeAnalysis.GetFocussedViewState();

	const u16* vram = huc6270->GetVRAM();
	const u16* colorTable = huc6260->GetColorTable();
	const FSpriteInfo* spriteInfo = pVRAMViewer->GetSpriteInfo();

	// Recreate texture if screen resolution changed
	GG_Runtime_Info info;
	pCore->GetRuntimeInfo(info);
	if (bRecreateTexture || info.screen_width != TextureWidth || info.screen_height != TextureHeight)
	{
		TextureWidth  = info.screen_width;
		TextureHeight = info.screen_height;
		delete[] ScreenBuffer;
		ScreenBuffer = new uint8_t[TextureWidth * TextureHeight * 4]();
		if (ScreenTexture)
			ImGui_FreeTexture(ScreenTexture);
		ScreenTexture = ImGui_CreateTextureRGBA(ScreenBuffer, TextureWidth, TextureHeight);
	}

	if (TextureWidth == 0 || TextureHeight == 0)
		return;

	// Sprite mode (affects bit-plane count) — use current register as fallback
	const bool mode1 = ((pHWState->R[HUC6270_REG_MWR] >> 2) & 0x03) == 1;

	// Use per-scanline snapshots when available, otherwise fall back to current registers
	const bool bHaveSnapshots = LastRenderedScanline >= 0;
	const uint16_t* satData = bHaveSnapshots ? FrameSAT : huc6270->GetSAT();

	// Expand 3-bit PCE colour component to 8-bit
	static const u8 kExpand3to8[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };

	// Renders one sprite pass. Only sprites where (priority flag set == behindBG) are drawn.
	// Iterate 63→0 so lower-index sprites composite last and appear on top.
	auto DrawSpritePass = [&](bool behindBG)
	{
		for (int s = HUC6270_SPRITES - 1; s >= 0; s--)
		{
			const int  sprite_offset = s << 2;
			const u16  flags         = satData[sprite_offset + 3] & 0xB98F;
			if (((flags & 0x0080) == 0) != behindBG) continue;

			const bool x_flip        = (flags & 0x0800) != 0;
			const bool y_flip        = (flags & 0x8000) != 0;
			const int  palette       = flags & 0x0F;
			const int  mode1_offset  = mode1 ? (satData[sprite_offset + 2] & 1) << 5 : 0;

			const int cgx = (flags >> 8) & 0x01;
			const int cgy = (flags >> 12) & 0x03;
			const int width  = k_huc6270_sprite_width[cgx];
			const int height = k_huc6270_sprite_height[cgy];
			u16 pattern = (satData[sprite_offset + 2] >> 1) & 0x3FF;
			pattern &= k_huc6270_sprite_mask_width[cgx];
			pattern &= k_huc6270_sprite_mask_height[cgy];
			const uint16_t sprite_address = pattern << 6;

			const int dest_y0 = (int)(satData[sprite_offset + 0] & 0x3FF) - 64;
			const int dest_x0 = (int)(satData[sprite_offset + 1] & 0x3FF) - 32;

			for (int py = 0; py < height; py++)
			{
				const int dest_y = dest_y0 + py;
				if (dest_y < 0 || dest_y >= TextureHeight) continue;

				const int flipped_y  = y_flip ? (height - 1 - py) : py;
				const int tile_y_idx = flipped_y >> 4;
				const int offset_y   = flipped_y & 0xF;
				const u16 line_start = sprite_address + tile_y_idx * 2 * 64 + offset_y;

				for (int px = 0; px < width; px++)
				{
					const int dest_x = dest_x0 + px;
					if (dest_x < 0 || dest_x >= TextureWidth) continue;

					const int flipped_x  = x_flip ? (width - 1 - px) : px;
					const int tile_x_idx = flipped_x >> 4;
					const int line = line_start + tile_x_idx * 64 + mode1_offset;

					if ((line + 48) >= HUC6270_VRAM_SIZE) continue;

					const u16 plane1 = vram[line];
					const u16 plane2 = vram[line + 16];
					const u16 plane3 = mode1 ? 0 : vram[line + 32];
					const u16 plane4 = mode1 ? 0 : vram[line + 48];

					const int pixel_x = 15 - (flipped_x & 0xF);
					const int color   = ((plane1 >> pixel_x) & 1)        |
					                    (((plane2 >> pixel_x) & 1) << 1) |
					                    (((plane3 >> pixel_x) & 1) << 2) |
					                    (((plane4 >> pixel_x) & 1) << 3);

					if (color == 0) continue;  // transparent

					const u16 cv  = colorTable[0x100 + palette * 16 + color];
					uint8_t*  dst = ScreenBuffer + (dest_y * TextureWidth + dest_x) * 4;
					dst[0] = kExpand3to8[(cv >> 3) & 0x07];
					dst[1] = kExpand3to8[(cv >> 6) & 0x07];
					dst[2] = kExpand3to8[cv & 0x07];
					dst[3] = 255;
				}
			}
		}
	};

	// --- Pass 1: Backdrop (global palette entry 0) ---
	if (bDrawPass[0])
	{
		const u16 cv = colorTable[0];
		const u8 r = kExpand3to8[(cv >> 3) & 0x07];
		const u8 g = kExpand3to8[(cv >> 6) & 0x07];
		const u8 b = kExpand3to8[cv & 0x07];
		uint8_t* dst = ScreenBuffer;
		for (int i = 0; i < TextureWidth * TextureHeight; i++, dst += 4)
		{ dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = 255; }
	}

	// CR/BG-enable fallback when no snapshots are available yet
	const bool bgEnabled  = (pHWState->R[HUC6270_REG_CR] & 0x0080) != 0;
	const bool sprEnabled = (pHWState->R[HUC6270_REG_CR] & 0x0040) != 0;

	// --- Pass 2: Behind-BG sprites (priority flag set) ---
	if (bDrawPass[1] &&sprEnabled && bDrawSprites)
		DrawSpritePass(true);

	// --- Pass 3: Background — per-scanline scroll when snapshots are available ---

	if (bDrawPass[2] && bgEnabled && bDrawBackground)
	for (int sy = 0; sy < TextureHeight; sy++)
	{
		// Use the per-scanline latched values if we have them; fall back to current registers
		uint16_t line_bxr, line_mwr;
		int32_t  line_byr_eff;
		if (bHaveSnapshots && sy <= LastRenderedScanline)
		{
			const FScanlineSnapshot& snap = ScanlineSnapshots[sy];
			line_bxr     = snap.bxr;
			line_byr_eff = snap.byr_eff;
			line_mwr     = snap.mwr;
		}
		else
		{
			line_bxr     = pHWState->R[HUC6270_REG_BXR] & 0x3FF;
			line_byr_eff = (pHWState->R[HUC6270_REG_BYR] & 0x1FF) + sy;
			line_mwr     = pHWState->R[HUC6270_REG_MWR];
		}

		const int screen_reg = (line_mwr >> 4) & 0x07;
		const int bat_w      = k_huc6270_screen_size_x[screen_reg];
		const int x_mask     = k_huc6270_screen_size_x_pixels_mask[screen_reg];
		const int y_mask     = k_huc6270_screen_size_y_pixels_mask[screen_reg];

		const int bg_y      = line_byr_eff & y_mask;
		const int tile_y    = bg_y >> 3;
		const int pixel_row = bg_y & 7;

		uint8_t* dst_row = ScreenBuffer + sy * TextureWidth * 4;

		for (int sx = 0; sx < TextureWidth; sx++)
		{
			const int bg_x   = (sx + line_bxr) & x_mask;
			const int tile_x = bg_x >> 3;
			const int bat_i  = tile_x + tile_y * bat_w;

			const u16 bat_entry      = vram[bat_i];
			const int tile_base      = (bat_entry & 0x07FF) * 16;
			const int color_tbl_base = ((bat_entry >> 12) & 0x0F) * 16;

			const u16 word_a = vram[tile_base + pixel_row];
			const u16 word_b = vram[tile_base + pixel_row + 8];
			const u8 byte1 = (u8)(word_a & 0xFF);
			const u8 byte2 = (u8)(word_a >> 8);
			const u8 byte3 = (u8)(word_b & 0xFF);
			const u8 byte4 = (u8)(word_b >> 8);
			const int shift = 7 - (bg_x & 7);
			const int color = ((byte1 >> shift) & 1)        |
			                  (((byte2 >> shift) & 1) << 1) |
			                  (((byte3 >> shift) & 1) << 2) |
			                  (((byte4 >> shift) & 1) << 3);

			if (color == 0) continue;  // transparent — show backdrop / behind-BG sprite

			const u16 cv  = colorTable[color_tbl_base + color];
			uint8_t*  dst = dst_row + sx * 4;
			dst[0] = kExpand3to8[(cv >> 3) & 0x07];
			dst[1] = kExpand3to8[(cv >> 6) & 0x07];
			dst[2] = kExpand3to8[cv & 0x07];
			dst[3] = 255;
		}
	}

	// --- Pass 4: In-front-of-BG sprites (priority flag clear) ---
	if (bDrawPass[3] && sprEnabled && bDrawSprites)
		DrawSpritePass(false);

	// --- Upload & display ---
	ImGui_UpdateTextureRGBA(ScreenTexture, ScreenBuffer);

	FGlobalConfig* pConfig = codeAnalysis.pGlobalConfig;
	ImGui::InputInt("Scale", &pConfig->GfxScreenScale, 1, 1);
	pConfig->GfxScreenScale = MAX(1, pConfig->GfxScreenScale);
	//ImGui::SameLine();
	ImGui::Checkbox("Background", &bDrawBackground);
	ImGui::SameLine();
	ImGui::Checkbox("Sprites", &bDrawSprites);
	const float scale = (float)pConfig->GfxScreenScale;

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::Image(ScreenTexture, ImVec2(TextureWidth * scale, TextureHeight * scale));

	// --- Scanline indicator: shows how far into the current frame the VDC has rendered ---
	if (bHaveSnapshots && LastRenderedScanline >= 0 && LastRenderedScanline < TextureHeight)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float lineY = pos.y + (LastRenderedScanline + 1) * scale;
		dl->AddLine(ImVec2(pos.x, lineY), ImVec2(pos.x + TextureWidth * scale, lineY), 0x80ffffff);
	}

	// --- Hover tooltip ---
	if (ImGui::IsItemHovered())
	{
		ImGuiIO& io = ImGui::GetIO();
		const int px = MIN(MAX((int)((io.MousePos.x - pos.x) / scale), 0), TextureWidth  - 1);
		const int py = MIN(MAX((int)((io.MousePos.y - pos.y) / scale), 0), TextureHeight - 1);

		ImDrawList* dl = ImGui::GetWindowDrawList();

		// Resolve scroll/size for the row under the cursor
		const FScanlineSnapshot& tipSnap = (bHaveSnapshots && py <= LastRenderedScanline)
		    ? ScanlineSnapshots[py]
		    : FScanlineSnapshot{ (uint16_t)(pHWState->R[HUC6270_REG_BXR] & 0x3FF),
		                         (int32_t)((pHWState->R[HUC6270_REG_BYR] & 0x1FF) + py),
		                         pHWState->R[HUC6270_REG_MWR], pHWState->R[HUC6270_REG_CR] };
		const int tip_screen_reg = (tipSnap.mwr >> 4) & 0x07;
		const int tip_bat_w      = k_huc6270_screen_size_x[tip_screen_reg];
		const int tip_x_mask     = k_huc6270_screen_size_x_pixels_mask[tip_screen_reg];
		const int tip_y_mask     = k_huc6270_screen_size_y_pixels_mask[tip_screen_reg];

		// Background tile under cursor
		const int bg_x  = (px + tipSnap.bxr) & tip_x_mask;
		const int bg_y  = tipSnap.byr_eff     & tip_y_mask;
		const int bat_i = (bg_x >> 3) + (bg_y >> 3) * tip_bat_w;
		const u16 bat_entry  = vram[bat_i];
		const int tile_idx   = bat_entry & 0x07FF;
		const int color_tbl  = (bat_entry >> 12) & 0x0F;
		const FAddressRef batWriter = pVRAMViewer->GetVRAMAccess(bat_i).LastWriter;

		// Highlight the 8x8 BG tile cell
		const float tx0 = pos.x + (float)(px & ~7) * scale;
		const float ty0 = pos.y + (float)(py & ~7) * scale;
		dl->AddRect(ImVec2(tx0, ty0), ImVec2(tx0 + 8.0f * scale, ty0 + 8.0f * scale), 0xffffffff);

		const uint16_t satBase = pHWState->R[HUC6270_REG_DVSSR];

		ImGui::BeginTooltip();
		ImGui::Text("Screen: (%d, %d)", px, py);
		ImGui::Separator();

		ImGui::Text("Background:");
		ImGui::Text("  BAT $%04X  tile $%03X  VRAM $%04X  palette %d", bat_i, tile_idx, tile_idx * 16, color_tbl);
		if (batWriter.IsValid())
		{
			ImGui::Text("  BAT writer:"); ImGui::SameLine();
			DrawAddressLabel(codeAnalysis, viewState, batWriter);
		}

		// Check each sprite for coverage of this pixel
		bool anySprite = false;
		for (int s = 0; s < HUC6270_SPRITES; s++)
		{
			const int sprite_offset = s << 2;
			const int spr_y0 = (int)(satData[sprite_offset + 0] & 0x3FF) - 64;
			const int spr_x0 = (int)(satData[sprite_offset + 1] & 0x3FF) - 32;
			const int w = spriteInfo[s].Width;
			const int h = spriteInfo[s].Height;

			if (px < spr_x0 || px >= spr_x0 + w || py < spr_y0 || py >= spr_y0 + h)
				continue;

			if (!anySprite)
			{
				ImGui::Separator();
				ImGui::Text("Sprites:");
				anySprite = true;
			}

			const FAddressRef yWriter   = pVRAMViewer->GetVRAMAccess(satBase + s * 4 + 0).LastWriter;
			const FAddressRef xWriter   = pVRAMViewer->GetVRAMAccess(satBase + s * 4 + 1).LastWriter;
			const FAddressRef patWriter = pVRAMViewer->GetVRAMAccess(satBase + s * 4 + 2).LastWriter;

			ImGui::Text("  [%d] pos (%d,%d)  %dx%d  VRAM $%04X", s, spr_x0, spr_y0, w, h, spriteInfo[s].Address);
			if (patWriter.IsValid()) { ImGui::Text("    gfx:"); ImGui::SameLine(); DrawAddressLabel(codeAnalysis, viewState, patWriter); }
			if (xWriter.IsValid())   { ImGui::Text("    x:  "); ImGui::SameLine(); DrawAddressLabel(codeAnalysis, viewState, xWriter);   }
			if (yWriter.IsValid())   { ImGui::Text("    y:  "); ImGui::SameLine(); DrawAddressLabel(codeAnalysis, viewState, yWriter);   }

			// Highlight sprite bounding box on the image
			const float sx0 = pos.x + spr_x0 * scale;
			const float sy0 = pos.y + spr_y0 * scale;
			dl->AddRect(ImVec2(sx0, sy0), ImVec2(sx0 + w * scale, sy0 + h * scale), 0xff00ffff, 0.f, 0, 2.f);
		}

		ImGui::EndTooltip();

		if (ImGui::IsMouseDoubleClicked(0) && batWriter.IsValid())
			viewState.GoToAddress(batWriter);
	}
}

#ifndef NDEBUG
int gPosX = 16;
int gPosY = 16;
int gXSize = 2;
int gYSize = 2;
int gOffset = 0;
int gPalette = 0;
extern bool gPCEGraphicsViewMonoSprites;
#endif

void FPCEGraphicsViewer::DrawTest()
{
#ifndef NDEBUG
	ImGui::SliderInt("x size (in 16x16 blocks)", &gXSize, 1, 256);
	ImGui::SliderInt("y size in (16x16 blocks)", &gYSize, 1, 256);
	ImGui::SliderInt("x", &gPosX, 0, 256 - 16);
	ImGui::SliderInt("y", &gPosY, 0, 256 - 16);
	ImGui::SliderInt("memory offset", &gOffset, 0, 0x2000);
	ImGui::InputInt("palette", &gPalette);
	ImGui::Checkbox("monochrome", &gPCEGraphicsViewMonoSprites);

	// rabio lepus rom 17 offset 0x1200
	FCodeAnalysisBank* pBank = pPCEEmu->GetCodeAnalysis().GetBank(100);
	if (pBank)
	{
		const uint8_t* ptr = pBank->Memory + 0x1200; // rabbit.32x32 bank 100
		//const uint8_t* ptr = pBank->Memory + 0x1100; // carrot.16x32 bank 100
		//const uint8_t* ptr = pBank->Memory + 0x1600; // bowtie. 16x16 bank 100
		//const uint8_t* ptr = pBank->Memory + 0x1980; // circle. 16x16 bank 100
		//const uint8_t* ptr = pBank->Memory + gOffset; // bowtie. 16x16 bank 100
		pTestPCEGraphicsView->Clear(0xfffff00);
		pTestPCEGraphicsView->Draw4bppSpriteImage(ptr, gPosX, gPosY, gXSize, gYSize, gPalette);
		pTestPCEGraphicsView->Draw();
	}
#endif // !NDEBUG
}