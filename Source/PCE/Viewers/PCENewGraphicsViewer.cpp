#include "PCENewGraphicsViewer.h"

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

// todo: update the graphic view when a new game is loaded
void FPCENewGraphicsViewer::UpdateGraphicView()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	const FCodeAnalysisBank* pBank = state.GetBank(SelectedBankId);
	if (pBank == nullptr || pBank->Memory == nullptr)
		return;

	const int blockBytes = (ViewMode == EPCEGraphicsViewMode::Sprites) ? kBytesPerSpriteBlock : kBytesPerBGTile;
	const int blockSizePixels = (ViewMode == EPCEGraphicsViewMode::Sprites) ? 16 : 8;

	int blocksPerRow = std::max(1, ViewWidth / blockSizePixels);
	int rows = std::max(1, ViewHeight / blockSizePixels);

	const int maxBlocks = pBank->GetSizeBytes() / blockBytes;
	if (blocksPerRow * rows > maxBlocks)
		rows = std::max(1, maxBlocks / blocksPerRow);

	const int width = blocksPerRow * blockSizePixels;
	const int height = rows * blockSizePixels;

	if (pGraphicView == nullptr || pGraphicView->GetWidth() != width || pGraphicView->GetHeight() != height)
	{
		delete pGraphicView;
		pGraphicView = new FGraphicsView(width, height);
	}

	pGraphicView->Clear(0xFF000000);
	if (ViewMode == EPCEGraphicsViewMode::Sprites)
		pGraphicView->Draw4bpp16x16PlanarSpriteImage(pBank->Memory, 0, 0, blocksPerRow, rows, GreyscalePalette);
	else
		pGraphicView->Draw4bpp8x8PlanarBGTileImage(pBank->Memory, 0, 0, blocksPerRow, rows, GreyscalePalette);
	pGraphicView->UpdateTexture();
}

FAddressRef FPCENewGraphicsViewer::GetAddressFromPos(int xp, int yp) const
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	const FCodeAnalysisBank* pBank = state.GetBank(SelectedBankId);
	if (pBank == nullptr || pGraphicView == nullptr/* || pBank->PrimaryMappedPage == -1*/)
		return FAddressRef();

	const int blockBytes = (ViewMode == EPCEGraphicsViewMode::Sprites) ? kBytesPerSpriteBlock : kBytesPerBGTile;
	const int blockSizePixels = (ViewMode == EPCEGraphicsViewMode::Sprites) ? 16 : 8;
	const int blocksPerRow = pGraphicView->GetWidth() / blockSizePixels;

	const int blockCol = xp / blockSizePixels;
	const int blockRow = yp / blockSizePixels;
	const int blockIndex = (blockRow * blocksPerRow) + blockCol;
	const int byteOffset = blockIndex * blockBytes;

	FAddressRef addr(SelectedBankId, pBank->GetMappedAddress());
	state.AdvanceAddressRef(addr, byteOffset);
	return addr;
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
		const int16_t bankId = pBankSet->GetBankId(0);
		if (std::find(BankIdsForBankCombo.begin(), BankIdsForBankCombo.end(), bankId) == BankIdsForBankCombo.end())
		{
			if (state.GetBank(bankId) != nullptr)
				BankIdsForBankCombo.emplace_back(bankId);
		}
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

	const FCodeAnalysisBank* pSelectedBank = state.GetBank(SelectedBankId);
	const char* pSelectedName = pSelectedBank != nullptr ? pSelectedBank->Name.c_str() : "None";

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

	const int blockSizePixels = (ViewMode == EPCEGraphicsViewMode::Sprites) ? 16 : 8;
	const int blockBytes = (ViewMode == EPCEGraphicsViewMode::Sprites) ? kBytesPerSpriteBlock : kBytesPerBGTile;
	const int maxBlocks = (pSelectedBank != nullptr) ? pSelectedBank->GetSizeBytes() / blockBytes : 0;

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

void FPCENewGraphicsViewer::GoToAddress(FAddressRef address)
{
}
