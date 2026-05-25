#include "VRAMViewer.h"
#include "SpriteViewer.h"

#include <string.h>
#include <imgui.h>

void DrawDataAcessIndicator(const ImVec2& pos, ImU32 fillCol, ImU32 brdCol, float lineHeight, float lh2);
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "CodeAnalyser/UI/UIColours.h"
#include "optick/optick.h"
#include "../PCEEmu.h"

#include "Misc/EmuBase.h"

static const int kMemoryViewImageWidth  = 128;
static const int kMemoryViewImageHeight = 256;

static const int kVRAMTileViewWidth    = 256;   // 32 tiles across
static const int kVRAMTileViewHeight   = 256;   // 32 tile rows = 1024 tiles visible
static const int kVRAMSpriteViewWidth  = 256;   // 16 sprite blocks across
static const int kVRAMSpriteViewHeight = 256;   // 16 sprite rows = 256 blocks visible

static const u8 kExpand3to8[8] = { 0, 36, 73, 109, 146, 182, 219, 255 };

// Build a 16-entry RGBA palette from the HuC6260 colour table.
// paletteBase is the colour table index of colour 0 for this palette.
static void BuildHWPalette(const u16* colorTable, int paletteBase, uint32_t* out)
{
	for (int i = 0; i < 16; i++)
	{
		const u16 cv = colorTable[paletteBase + i];
		const u8 r = kExpand3to8[(cv >> 3) & 7];
		const u8 g = kExpand3to8[(cv >> 6) & 7];
		const u8 b = kExpand3to8[cv & 7];
		out[i] = 0xFF000000u | ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
	}
}

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

	BGTileViewImage = new FGraphicsView(kVRAMTileViewWidth, kVRAMTileViewHeight);
	BGTileViewImage->Clear(0xff000000);

	SpriteViewImage = new FGraphicsView(kVRAMSpriteViewWidth, kVRAMSpriteViewHeight);
	SpriteViewImage->Clear(0xff000000);

	return true;
}

void FVRAMViewer::Reset(void)
{
	ClearUsage();
	memset(SpriteIndexLookup, -1, sizeof(SpriteIndexLookup));
}

void FVRAMViewer::ClearUsage(void)
{
	for (FVRAMAccess& access : Access)
	{
		access.FrameLastWritten = -1;
		access.FrameLastRead    = -1;
		access.LastWriter       = FAddressRef::Invalid();
		access.LastReader       = FAddressRef::Invalid();
	}
	LastVRAMWriter     = FAddressRef::Invalid();
	LastVRAMWriteFrame = -1;
	LastVRAMReader     = FAddressRef::Invalid();
	LastVRAMReadFrame  = -1;
}

void FVRAMViewer::DrawUI(void)
{
	if (ImGui::BeginTabBar("VRAMViewerTabs"))
	{
		if (ImGui::BeginTabItem("Map"))
		{
			DrawPhysicalMemoryOverview();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("BG Tiles"))
		{
			DrawBGTileView();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Sprites"))
		{
			DrawSpriteView();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void FVRAMViewer::DrawBGTileView()
{
	HuC6270* huc6270   = pPCEEmu->GetCore()->GetHuC6270_1();
	HuC6260* huc6260   = pPCEEmu->GetCore()->GetHuC6260();
	const u16* vram       = huc6270->GetVRAM();
	const u16* colorTable = huc6260->GetColorTable();

	ImGui::SliderInt("Palette##bgtile", &BGTilePalette, 0, 15);
	ImGui::InputInt("Scale##bgtile",    &BGTileScale, 1, 1);
	BGTileScale = MAX(1, BGTileScale);

	constexpr int kTilesPerRow  = kVRAMTileViewWidth  / 8;  // 32
	constexpr int kTileRowCount = kVRAMTileViewHeight / 8;  // 32
	constexpr int kTilesVisible = kTilesPerRow * kTileRowCount;   // 1024
	constexpr int kMaxTiles     = HUC6270_VRAM_SIZE / 16;         // 2048

	BGTileOffset = MAX(0, MIN(BGTileOffset, kMaxTiles - kTilesVisible));

	uint32_t palette[16];
	BuildHWPalette(colorTable, BGTilePalette * 16, palette);

	BGTileViewImage->Clear(0xFF000000);
	const int tilesAvailable = MIN(kTilesVisible, kMaxTiles - BGTileOffset);
	const int rowsToDraw     = tilesAvailable / kTilesPerRow;
	if (rowsToDraw > 0)
	{
		const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(vram + BGTileOffset * 16);
		BGTileViewImage->Draw4bpp8x8PlanarBGTileImage(pSrc, 0, 0, kTilesPerRow, rowsToDraw, palette);
	}
	BGTileViewImage->UpdateTexture();

	const float scale = ImGui_GetScaling() * (float)BGTileScale;
	const ImVec2 imageSize(kVRAMTileViewWidth * scale, kVRAMTileViewHeight * scale);
	ImGui::Image((void*)BGTileViewImage->GetTexture(), imageSize);

	if (ImGui::IsItemHovered())
	{
		const float wheel = ImGui::GetIO().MouseWheel;
		BGTileOffset = MAX(0, MIN(BGTileOffset - (int)wheel * kTilesPerRow, kMaxTiles - kTilesVisible));
	}

	ImGui::SliderInt("Tile##bgtileoffset", &BGTileOffset, 0, kMaxTiles - kTilesVisible);
}

void FVRAMViewer::DrawSpriteView()
{
	HuC6270* huc6270   = pPCEEmu->GetCore()->GetHuC6270_1();
	HuC6260* huc6260   = pPCEEmu->GetCore()->GetHuC6260();
	const u16* vram       = huc6270->GetVRAM();
	const u16* colorTable = huc6260->GetColorTable();

	ImGui::SliderInt("Palette##spr", &SpritePalette, 0, 15);
	ImGui::InputInt("Scale##spr",    &SpriteScale, 1, 1);
	SpriteScale = MAX(1, SpriteScale);

	constexpr int kSpritesPerRow  = kVRAMSpriteViewWidth  / 16; // 16
	constexpr int kSpriteRowCount = kVRAMSpriteViewHeight / 16; // 16
	constexpr int kBlocksVisible  = kSpritesPerRow * kSpriteRowCount;  // 256
	constexpr int kMaxBlocks      = HUC6270_VRAM_SIZE / 64;            // 512

	SpriteBlockOffset = MAX(0, MIN(SpriteBlockOffset, kMaxBlocks - kBlocksVisible));

	// Sprite palettes occupy the second half of the colour table (offset 0x100)
	uint32_t palette[16];
	BuildHWPalette(colorTable, 0x100 + SpritePalette * 16, palette);

	SpriteViewImage->Clear(0xFF000000);
	const int blocksAvailable = MIN(kBlocksVisible, kMaxBlocks - SpriteBlockOffset);
	const int rowsToDraw      = blocksAvailable / kSpritesPerRow;
	if (rowsToDraw > 0)
	{
		const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(vram + SpriteBlockOffset * 64);
		SpriteViewImage->Draw4bpp16x16PlanarSpriteImage(pSrc, 0, 0, kSpritesPerRow, rowsToDraw, palette);
	}
	SpriteViewImage->UpdateTexture();

	const float scale = ImGui_GetScaling() * (float)SpriteScale;
	const ImVec2 imageSize(kVRAMSpriteViewWidth * scale, kVRAMSpriteViewHeight * scale);
	ImGui::Image((void*)SpriteViewImage->GetTexture(), imageSize);

	if (ImGui::IsItemHovered())
	{
		const float wheel = ImGui::GetIO().MouseWheel;
		SpriteBlockOffset = MAX(0, MIN(SpriteBlockOffset - (int)wheel * kSpritesPerRow, kMaxBlocks - kBlocksVisible));
	}

	ImGui::SliderInt("Block##sproffset", &SpriteBlockOffset, 0, kMaxBlocks - kBlocksVisible);
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
	LegendRow("Unwritten",    kUnwrittenCol,          "Never accessed");
	LegendRow("Write",        kUnknownWriteCol,        "Written");
	LegendRow("WriteActive",  kUnknownWriteActiveCol,  "Written (active)");
	LegendRow("Read",         kDataReadCol,            "Read");
	LegendRow("ReadActive",   kDataReadActiveCol,      "Read (active)");
	ImGui::EndTooltip();
}

void	FVRAMViewer::DrawPhysicalMemoryOverview()
{
	FCodeAnalysisState& state     = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	MemoryViewImage->Clear(0xff808080);

	uint32_t* pViewImagePixels = MemoryViewImage->GetPixelBuffer();
	uint32_t* pPix = pViewImagePixels;

	DrawUtilisationMap(state,pPix);

	FGlobalConfig* pConfig = state.pGlobalConfig;

	ImGui::InputInt("Scale", &pConfig->VRAMViewerScale, 1, 1);
	pConfig->VRAMViewerScale = MAX(1, pConfig->VRAMViewerScale);	// clamp
	ImGui::SameLine();
	if (ImGui::Button("Clear Usage"))
		ClearUsage();
	
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
		DrawLegend();

	if (bMapIsHovered)
	{
		const int xp = (int)((io.MousePos.x - pos.x) / scale);
		const int yp = (int)((io.MousePos.y - pos.y) / scale);
		const uint16_t addr = (uint16_t)(xp + yp * kMemoryViewImageWidth);

		ImGui::Text("VRAM: $%04X", addr);

		const FVRAMAccess& access = Access[addr];
		if (access.LastWriter.IsValid())
		{
			ImGui::Text("Written by:"); ImGui::SameLine();
			DrawAddressLabel(state, viewState, access.LastWriter);
		}
		if (access.LastReader.IsValid())
		{
			ImGui::Text("Read by:"); ImGui::SameLine();
			DrawAddressLabel(state, viewState, access.LastReader);
		}

		SpriteHighlight = GetSpriteIndexForAddress(addr);

		if (ImGui::IsMouseDoubleClicked(0) && access.LastWriter.IsValid())
			viewState.GoToAddress(access.LastWriter, false);
	}
	else
	{
		ImGui::NewLine();
	}

	// Last writer / reader activity rows
	const float lineHeight = ImGui::GetTextLineHeight();
	const float lh2        = (float)(int)(lineHeight / 2.0f);
	const int   curFrame   = state.CurrentFrameNo;
	const ImU32 brdCol     = 0xFF000000;

	{
		const int framesSince = LastVRAMWriteFrame == -1 ? 255 : curFrame - LastVRAMWriteFrame;
		const int brightness  = (255 - MIN(framesSince << 2, 255)) & 0xff;
		const ImVec2 indPos   = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(14.0f, lineHeight));
		if (brightness > 0)
			DrawDataAcessIndicator(indPos, 0xff000000 | brightness, brdCol, lineHeight, lh2);
		ImGui::SameLine();
		ImGui::Text("Last write:");
		ImGui::SameLine();
		if (LastVRAMWriter.IsValid())
			DrawAddressLabel(state, viewState, LastVRAMWriter);
		else
			ImGui::TextDisabled("None");
	}
	{
		const int framesSince = LastVRAMReadFrame == -1 ? 255 : curFrame - LastVRAMReadFrame;
		const int brightness  = (255 - MIN(framesSince << 2, 255)) & 0xff;
		const ImVec2 indPos   = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(14.0f, lineHeight));
		if (brightness > 0)
			DrawDataAcessIndicator(indPos, 0xff000000 | (brightness << 8), brdCol, lineHeight, lh2);
		ImGui::SameLine();
		ImGui::Text("Last read: ");
		ImGui::SameLine();
		if (LastVRAMReader.IsValid())
			DrawAddressLabel(state, viewState, LastVRAMReader);
		else
			ImGui::TextDisabled("None");
	}
}

void FVRAMViewer::DrawUtilisationMap(FCodeAnalysisState& state, uint32_t* pPix)
{
	const int frameThreshold = 4;
	const int currentFrameNo = state.CurrentFrameNo;

	for (uint16_t addr = 0; addr < HUC6270_VRAM_SIZE; addr++)
	{
		const FVRAMAccess& access   = Access[addr];
		const bool written          = access.FrameLastWritten != -1;
		const bool activeWrite      = written    && (currentFrameNo - access.FrameLastWritten < frameThreshold);
		const bool read             = access.FrameLastRead != -1;
		const bool activeRead       = read       && (currentFrameNo - access.FrameLastRead    < frameThreshold);
		const int  spriteIndex      = SpriteIndexLookup[addr];

		uint32_t drawCol = kUnwrittenCol;

		if (activeWrite)
			drawCol = kUnknownWriteActiveCol;
		else if (written)
			drawCol = kUnknownWriteCol;
		else if (activeRead)
			drawCol = kDataReadActiveCol;
		else if (read)
			drawCol = kDataReadCol;

		if (SpriteHighlight != -1 && spriteIndex == SpriteHighlight)
			drawCol = Colours::GetFlashColour();

		*pPix++ = drawCol;
	}
}

void FVRAMViewer::RegisterRead(uint16_t vramAddress, FAddressRef reader)
{
	if (vramAddress < HUC6270_VRAM_SIZE)
	{
		const int frameNo        = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
		FVRAMAccess& access      = Access[vramAddress];
		access.FrameLastRead     = frameNo;
		access.LastReader        = reader;
		LastVRAMReader           = reader;
		LastVRAMReadFrame        = frameNo;
	}
}

void FVRAMViewer::RegisterWrite(uint16_t vramAddress, FAddressRef writer)
{
	if (vramAddress < HUC6270_VRAM_SIZE)
	{
		const int frameNo        = pPCEEmu->GetCodeAnalysis().CurrentFrameNo;
		FVRAMAccess& access      = Access[vramAddress];
		access.FrameLastWritten  = frameNo;
		access.LastWriter        = writer;
		LastVRAMWriter           = writer;
		LastVRAMWriteFrame       = frameNo;
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
		SpriteInfo[i].SizeInBytes = (SpriteInfo[i].Width * SpriteInfo[i].Height) >> 1;
		SpriteInfo[i].Palette = flags & 0x0F;

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
		const uint16_t end   = start + SpriteInfo[i].SizeInBytes / 2; // this is in words, not bytes
		for (uint16_t addr = start + 1; addr < end && addr < HUC6270_VRAM_SIZE; addr++)
			SpriteIndexLookup[addr] = (int16_t)i;
	}

}

// todo: deal with the fact there can be multiple sprites sharing the same adddress.
// they can have the same address but different palettes
int FVRAMViewer::GetSpriteIndexForAddress(uint16_t addr) const
{
	return SpriteIndexLookup[addr];
}
