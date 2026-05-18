#include "MemoryViewer.h"

#include <imgui.h>
#include "../PCEEmu.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "huc6270_defines.h"

FMemoryViewer::FMemoryViewer(FEmuBase* pEmu)
	: FViewerBase(pEmu)
{
	Name    = "Memory Viewer";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FMemoryViewer::Init()
{
	Editor.ReadOnly = true;
	return true;
}

void FMemoryViewer::BuildMemoryList()
{
	Regions.clear();

	// VRAM — u16* treated as raw bytes
	GeargrafxCore* pCore = pPCEEmu->GetCore();
	if (pCore)
	{
		HuC6270* pVDC = pCore->GetHuC6270_1();
		if (pVDC)
		{
			FMemViewRegion& vram = Regions.emplace_back();
			vram.Name     = "VRAM";
			vram.Data     = pVDC->GetVRAM();
			vram.Size     = HUC6270_VRAM_SIZE * sizeof(u16);
			vram.BaseAddr = 0;
		}
	}

	// All visible banks
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	const FCodeAnalysisBank* banks = state.GetBanks();
	const int bankCount = FCodeAnalysisState::BankCount;
	for (int i = 0; i < bankCount; i++)
	{
		const FCodeAnalysisBank& bank = banks[i];
		if (bank.bHidden || bank.Memory == nullptr || bank.NoPages == 0 || !pPCEEmu->IsBankIdCanonical(bank.Id))
			continue;

		FMemViewRegion& region = Regions.emplace_back();
		region.Name     = bank.Name;
		region.Data     = bank.Memory;
		region.Size     = static_cast<size_t>(bank.NoPages) * 1024;
		region.BaseAddr = 0;
	}
}

void FMemoryViewer::DrawUI()
{
	BuildMemoryList();

	if (Regions.empty())
	{
		ImGui::TextDisabled("No memory regions available.");
		return;
	}

	if (SelectedRegion >= static_cast<int>(Regions.size()))
		SelectedRegion = 0;

	const FMemViewRegion& current = Regions[SelectedRegion];
	ImGui::SetNextItemWidth(200.0f);
	if (ImGui::BeginCombo("##memregion", current.Name.c_str()))
	{
		for (int i = 0; i < static_cast<int>(Regions.size()); i++)
		{
			const bool bSelected = (i == SelectedRegion);
			if (ImGui::Selectable(Regions[i].Name.c_str(), bSelected))
				SelectedRegion = i;
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	const FMemViewRegion& region = Regions[SelectedRegion];
	if (region.Data != nullptr)
		Editor.DrawContents(region.Data, region.Size, region.BaseAddr);
}
