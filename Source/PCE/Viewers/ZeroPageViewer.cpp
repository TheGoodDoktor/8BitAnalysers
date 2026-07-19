#include "ZeroPageViewer.h"

#include <algorithm>
#include <imgui.h>
#include <vector>

#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "../PCEEmu.h"
#include "../BankSet.h"

static constexpr uint16_t kZeroPageStart = 0x2000;
static constexpr uint16_t kZeroPageEnd   = 0x20FF;
static constexpr uint8_t  kWRAM0BankIndex = 0xf8;

FZeroPageViewer::FZeroPageViewer(FEmuBase* pEmu)
: FViewerBase(pEmu)
{
	Name = "Zero Page";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FZeroPageViewer::Init()
{
	return true;
}

void FZeroPageViewer::DrawUI()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	ImGui::Checkbox("Hide Unused Locations", &bHideUnused);
	if (bHideUnused)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.0f);
		ImGui::InputInt("Frame limit", &unusedFrameLimit, 10);
		if (unusedFrameLimit < 1)
			unusedFrameLimit = 1;
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			resetFrameNo        = state.CurrentFrameNo;
			resetExecutionCount = state.ExecutionCounter;
		}
	}

	const int16_t wramBankId = pPCEEmu->GetBankSet(kWRAM0BankIndex).GetBankId();
	const FCodeAnalysisBank* pBank = state.GetBank(wramBankId);
	if (pBank == nullptr)
		return;

	int numLocationsDisplayed = 0;

	std::vector<const FCodeAnalysisItem*> zeroPageItems;
	for (const FCodeAnalysisItem& item : pBank->ItemList)
	{
		const uint16_t addr = item.AddressRef.GetAddress();
		if (addr > kZeroPageEnd)
			break;
		if (addr < kZeroPageStart)
			continue;

		if (bHideUnused)
		{
			const FDataInfo* pDataInfo = state.GetDataInfoForAddress(item.AddressRef);
			if (pDataInfo == nullptr)
				continue;
			const int lastFrameAccessed = std::max(pDataInfo->LastFrameRead, pDataInfo->LastFrameWritten);
			if (lastFrameAccessed == -1 || (state.CurrentFrameNo - lastFrameAccessed) > unusedFrameLimit)
				continue;
			if (resetExecutionCount != -1)
			{
				const int lastExecAccessed = std::max(pDataInfo->LastRead, pDataInfo->LastWritten);
				if (lastExecAccessed <= resetExecutionCount)
					continue;
			}
		}

		if (item.Item->Type == EItemType::Data) // will zero page ever contain code?
			numLocationsDisplayed++;

		zeroPageItems.push_back(&item);
	}

	ImGui::Text("Showing: %d locations", numLocationsDisplayed);
	ImGui::Separator();

	for (const FCodeAnalysisItem* pItem : zeroPageItems)
	{
		DrawCodeAnalysisItem(state, viewState, *pItem);
		if (pItem->Item->Type == EItemType::Label)
			ImGui::SameLine();
	}

}
