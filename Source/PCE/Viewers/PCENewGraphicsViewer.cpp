#include "PCENewGraphicsViewer.h"

#include <cstdio>
#include <imgui.h>

#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "Util/GraphicsView.h"
#include "Util/Misc.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "../PCEEmu.h"
#include "../BankSet.h"

static const int kBytesPerSpriteBlock = 128;	// 4bpp 16x16 planar sprite
static const int kBytesPerBGTile      = 32;	// 4bpp 8x8 planar BG tile

static const int16_t kBankWRAM0 = 0xf8;

static void BuildGreyscalePalette(uint32_t* out)
{
	for (int i = 0; i < 16; i++)
	{
		const uint8_t v = (uint8_t)(i * 17);
		out[i] = 0xFF000000u | ((uint32_t)v << 16) | ((uint32_t)v << 8) | v;
	}
}

FPCENewGraphicsViewer::FPCENewGraphicsViewer(FEmuBase* pEmu)
	: FGraphicsViewerBase(pEmu)
{
	Name = "PCE Graphics";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);

	BuildGreyscalePalette(GreyscalePalette);

	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	
	BankIdsForBankCombo.reserve(128);
}

bool FPCENewGraphicsViewer::Init(void)
{
	return true;
}

void FPCENewGraphicsViewer::Shutdown(void)
{
	delete pGraphicView;
	pGraphicView = nullptr;
}

// todo: update the graphic view when a new game is loaded.
// todo: update the graphic view for RW memory on a new frame.
void FPCENewGraphicsViewer::UpdateGraphicView()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	const uint8_t* pSrc = nullptr;
	int sizeBytes = 0;

	if (MemorySource == EPCEMemorySource::VRAM)
	{
		HuC6270* huc6270 = pPCEEmu->GetCore()->GetHuC6270_1();
		pSrc = reinterpret_cast<const uint8_t*>(huc6270->GetVRAM());
		sizeBytes = HUC6270_VRAM_SIZE * sizeof(u16);
	}
	else
	{
		const int16_t bankId = (MemorySource == EPCEMemorySource::WRAM) ? WRAMBankId : SelectedBankId;
		const FCodeAnalysisBank* pBank = state.GetBank(bankId);
		if (pBank == nullptr || pBank->Memory == nullptr)
			return;

		pSrc = pBank->Memory;
		sizeBytes = pBank->GetSizeBytes();
	}

	const int blockBytes = (ViewMode == EPCEGraphicsViewMode::Sprites) ? kBytesPerSpriteBlock : kBytesPerBGTile;
	const int blockSizePixels = (ViewMode == EPCEGraphicsViewMode::Sprites) ? 16 : 8;

	int blocksPerRow = std::max(1, ViewWidth / blockSizePixels);
	int rows = std::max(1, ViewHeight / blockSizePixels);

	const int maxBlocks = sizeBytes / blockBytes;
	if (blocksPerRow * rows > maxBlocks)
		rows = std::max(1, maxBlocks / blocksPerRow);

	const int width = blocksPerRow * blockSizePixels;
	const int height = rows * blockSizePixels;

	if (pGraphicView == nullptr || pGraphicView->GetWidth() != width || pGraphicView->GetHeight() != height)
	{
		delete pGraphicView;
		pGraphicView = new FGraphicsView(width, height);
	}

	const uint32_t* pPalette = GetPaletteFromPaletteNo(SelectedPaletteIndex);
	if (pPalette == nullptr)
		pPalette = GreyscalePalette;

	pGraphicView->Clear(0xFF000000);
	if (ViewMode == EPCEGraphicsViewMode::Sprites)
		pGraphicView->Draw4bpp16x16PlanarSpriteImage(pSrc, 0, 0, blocksPerRow, rows, pPalette);
	else
		pGraphicView->Draw4bpp8x8PlanarBGTileImage(pSrc, 0, 0, blocksPerRow, rows, pPalette);
	pGraphicView->UpdateTexture();
}

FAddressRef FPCENewGraphicsViewer::GetAddressFromPos(int xp, int yp) const
{
	if (MemorySource == EPCEMemorySource::VRAM)
		return FAddressRef();

	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	const int16_t bankId = (MemorySource == EPCEMemorySource::WRAM) ? WRAMBankId : SelectedBankId;
	const FCodeAnalysisBank* pBank = state.GetBank(bankId);
	if (pBank == nullptr || pGraphicView == nullptr)
		return FAddressRef();

	const int blockBytes = (ViewMode == EPCEGraphicsViewMode::Sprites) ? kBytesPerSpriteBlock : kBytesPerBGTile;
	const int blockSizePixels = (ViewMode == EPCEGraphicsViewMode::Sprites) ? 16 : 8;
	const int blocksPerRow = pGraphicView->GetWidth() / blockSizePixels;

	const int blockCol = xp / blockSizePixels;
	const int blockRow = yp / blockSizePixels;
	const int blockIndex = (blockRow * blocksPerRow) + blockCol;
	const int byteOffset = blockIndex * blockBytes;

	FAddressRef addr(bankId, pBank->GetMappedAddress());
	state.AdvanceAddressRef(addr, byteOffset);
	return addr;
}

int FPCENewGraphicsViewer::GetVRAMOffsetFromPos(int xp, int yp) const
{
	if (pGraphicView == nullptr)
		return -1;

	const int blockBytes = (ViewMode == EPCEGraphicsViewMode::Sprites) ? kBytesPerSpriteBlock : kBytesPerBGTile;
	const int blockSizePixels = (ViewMode == EPCEGraphicsViewMode::Sprites) ? 16 : 8;
	const int blocksPerRow = pGraphicView->GetWidth() / blockSizePixels;

	const int blockCol = xp / blockSizePixels;
	const int blockRow = yp / blockSizePixels;
	const int blockIndex = (blockRow * blocksPerRow) + blockCol;
	return blockIndex * blockBytes;
}

void FPCENewGraphicsViewer::PopulateBankList(const FCodeAnalysisState& state)
{
	BankIdsForBankCombo.clear();

	// ROM banks
	for (int i = 0; i < FPCEEmu::kNumRomBanks; i++)
	{
		FBankSet* pBankSet = pPCEEmu->Banks[i];
		const int16_t bankId = pBankSet->GetBankId(0);
		if (std::find(BankIdsForBankCombo.begin(), BankIdsForBankCombo.end(), bankId) == BankIdsForBankCombo.end())
		{
			if (state.GetBank(bankId) != nullptr)
				BankIdsForBankCombo.emplace_back(bankId);
		}
	}

	// WRAM
	{
		FBankSet* pBankSet = pPCEEmu->Banks[kBankWRAM0];
		WRAMBankId = pBankSet->GetBankId(0);
	}

	if (!BankIdsForBankCombo.empty())
	{
		if (SelectedBankIndex < 0 || SelectedBankIndex >= BankIdsForBankCombo.size())
		{
			SelectedBankIndex = 0;
			SelectedBankId = BankIdsForBankCombo[0];
			bGraphicViewDirty = true;
		}
	}
}

void FPCENewGraphicsViewer::DrawUI(void)
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	PopulateBankList(state);

	EPCEGraphicsViewMode newViewMode = ViewMode;
	if (ImGui::RadioButton("Sprites", ViewMode == EPCEGraphicsViewMode::Sprites))
		newViewMode = EPCEGraphicsViewMode::Sprites;
	ImGui::SameLine();
	if (ImGui::RadioButton("BG Tiles", ViewMode == EPCEGraphicsViewMode::BGTiles))
		newViewMode = EPCEGraphicsViewMode::BGTiles;

	if (newViewMode != ViewMode)
	{
		ViewMode = newViewMode;
		bGraphicViewDirty = true;
	}

	static const char* kMemorySourceNames[] = { "ROM", "WRAM", "VRAM" };
	int memorySourceIndex = (int)MemorySource;
	if (ImGui::Combo("Memory", &memorySourceIndex, kMemorySourceNames, IM_ARRAYSIZE(kMemorySourceNames)))
	{
		MemorySource = (EPCEMemorySource)memorySourceIndex;
		bGraphicViewDirty = true;
	}

	const bool bBankSelectable = (MemorySource == EPCEMemorySource::ROM);

	const FCodeAnalysisBank* pSelectedBank = state.GetBank(SelectedBankId);
	const char* pSelectedName = pSelectedBank != nullptr ? pSelectedBank->Name.c_str() : "None";

	ImGui::BeginDisabled(!bBankSelectable);

	//ImGui::SetNextItemWidth(200);
	if (ImGui::BeginCombo("Bank", pSelectedName))
	{
		for (int i = 0; i < (int)BankIdsForBankCombo.size(); i++)
		{
			const int16_t bankId = BankIdsForBankCombo[i];
			const FCodeAnalysisBank* pBank = state.GetBank(bankId);
			if (pBank == nullptr)
				continue;

			const bool bSelected = (bankId == SelectedBankId);
			if (ImGui::Selectable(pBank->Name.c_str(), bSelected))
			{
				SelectedBankId = bankId;
				SelectedBankIndex = i;
				bGraphicViewDirty = true;
			}
		}

		ImGui::EndCombo();
	}

	//ImGui::SameLine();

	ImGui::PushButtonRepeat(true);
	if (ImGui::Button("Prev"))
	{
		if (SelectedBankIndex > 0)
		{
			SelectedBankIndex--;
			SelectedBankId = BankIdsForBankCombo[SelectedBankIndex];
			bGraphicViewDirty = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Next"))
	{
		if (SelectedBankIndex >= 0 && SelectedBankIndex < (int)BankIdsForBankCombo.size() - 1)
		{
			SelectedBankIndex++;
			SelectedBankId = BankIdsForBankCombo[SelectedBankIndex];
			bGraphicViewDirty = true;
		}
	}
	ImGui::PopButtonRepeat();

	ImGui::EndDisabled();

	const int blockSizePixels = (ViewMode == EPCEGraphicsViewMode::Sprites) ? 16 : 8;
	const int blockBytes = (ViewMode == EPCEGraphicsViewMode::Sprites) ? kBytesPerSpriteBlock : kBytesPerBGTile;

	int sourceSizeBytes = 0;
	if (MemorySource == EPCEMemorySource::VRAM)
		sourceSizeBytes = HUC6270_VRAM_SIZE * sizeof(u16);
	else if (MemorySource == EPCEMemorySource::WRAM)
	{
		const FCodeAnalysisBank* pWRAMBank = state.GetBank(WRAMBankId);
		sourceSizeBytes = (pWRAMBank != nullptr) ? pWRAMBank->GetSizeBytes() : 0;
	}
	else
	{
		sourceSizeBytes = (pSelectedBank != nullptr) ? pSelectedBank->GetSizeBytes() : 0;
	}
	const int maxBlocks = sourceSizeBytes / blockBytes;

	if (ImGui::InputInt("View Width", &ViewWidth, blockSizePixels, blockSizePixels))
		bGraphicViewDirty = true;
	ViewWidth = std::max(blockSizePixels, ViewWidth);

	if (ImGui::InputInt("View Height", &ViewHeight, blockSizePixels, blockSizePixels))
		bGraphicViewDirty = true;
	ViewHeight = std::max(blockSizePixels, ViewHeight);

	// Clamp width/height so they can't be increased beyond what's needed to show the whole bank
	if (maxBlocks > 0)
	{
		const int blocksPerRow = std::max(1, ViewWidth / blockSizePixels);
		const int maxRows = std::max(1, maxBlocks / blocksPerRow);
		ViewHeight = std::min(ViewHeight, maxRows * blockSizePixels);

		const int rows = std::max(1, ViewHeight / blockSizePixels);
		const int maxCols = std::max(1, maxBlocks / rows);
		ViewWidth = std::min(ViewWidth, maxCols * blockSizePixels);
	}

	if (bGraphicViewDirty)
	{
		UpdateGraphicView();
		bGraphicViewDirty = false;
	}

	ImGui::InputInt("Scale", &GraphicViewScale, 1, 1);
	GraphicViewScale = MAX(1, GraphicViewScale);

	DrawPaletteListBox();

	if (pGraphicView != nullptr)
	{
		const float scale = ImGui_GetScaling() * (float)GraphicViewScale;
		const ImVec2 imageSize((float)pGraphicView->GetWidth() * scale, (float)pGraphicView->GetHeight() * scale);
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Image((void*)pGraphicView->GetTexture(), imageSize);

		if (ImGui::IsItemHovered())
		{
			const ImVec2& mousePos = ImGui::GetIO().MousePos;
			const int xp = (int)((mousePos.x - pos.x) / scale);
			const int yp = (int)((mousePos.y - pos.y) / scale);

			if (MemorySource == EPCEMemorySource::VRAM)
			{
				const int vramOffset = GetVRAMOffsetFromPos(xp, yp);
				if (vramOffset >= 0)
				{
					ImGui::BeginTooltip();
					ImGui::Text("VRAM: %s", NumStr((uint16_t)vramOffset));
					ImGui::EndTooltip();
				}
			}
			else
			{
				const FAddressRef addr = GetAddressFromPos(xp, yp);
				if (addr.IsValid())
				{
					ImGui::BeginTooltip();
					ImGui::Text("%s", NumStr(addr.GetAddress()));
					ImGui::SameLine();
					DrawAddressLabel(state, state.GetFocussedViewState(), addr);
					ImGui::EndTooltip();
				}
			}
		}
	}

}

void FPCENewGraphicsViewer::DrawPaletteListBox()
{
	if (ImGui::BeginListBox("Palette", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 8)))
	{
		const int numPalettes = GetNoPaletteEntries();
		for (int p = 0; p < numPalettes; p++)
		{
			const FPaletteEntry* pEntry = GetPaletteEntry(p);
			if (pEntry == nullptr || pEntry->NoColours != 16)
				continue;

			const uint32_t* pPalette = GetPaletteFromPaletteNo(p);
			if (pPalette == nullptr)
				continue;

			char paletteName[16];
			if (p < 16)
				snprintf(paletteName, sizeof(paletteName), "BGND %02d", p);
			else if (p < 32)
				snprintf(paletteName, sizeof(paletteName), "SPRT %02d", p - 16);
			else
				snprintf(paletteName, sizeof(paletteName), "USER %02d", p - 32);

			if (ImGui::Selectable(paletteName, p == SelectedPaletteIndex))
			{
				SelectedPaletteIndex = p;
				bGraphicViewDirty = true;
			}

			ImGui::SameLine();
			DrawPalette(pPalette, 16);
		}

		ImGui::EndListBox();
	}
}

void FPCENewGraphicsViewer::GoToAddress(FAddressRef address)
{
}
