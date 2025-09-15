#include "DebugStatsViewer.h"

//#include <algorithm>

#include <imgui.h>

#include "../PCEEmu.h"
//#include <geargrafx_core.h>


FDebugStatsViewer::FDebugStatsViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Debug Stats";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FDebugStatsViewer::Init()
{
	return true;
}

void FDebugStatsViewer::DrawUI()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	
	int mappedBanks = 0;
	int banksWithPrimaryMappedPage = 0;
	int usedBanks = 0;

	auto& banks = state.GetBanks();
	for (auto& bank : banks)
	{
		if (bank.PrimaryMappedPage != -1)
			banksWithPrimaryMappedPage++;
		if (bank.IsMapped())
			mappedBanks++;
		if (bank.IsUsed())
			usedBanks++;
	}

	int pagesInUse = 0;
	for (int p = 0; p < state.GetNoPages(); p++)
	{
		FCodeAnalysisPage* pPage = state.GetPage(p);
		if (pPage->bUsed)
			pagesInUse++;
	}

	ImGui::SeparatorText("Banks");
	ImGui::Text("Total banks: %d", Banks.size());
	ImGui::Text("Mapped banks: %d", mappedBanks);
	ImGui::Text("Banks with primary mapped page: %d", banksWithPrimaryMappedPage);
	ImGui::Text("Used banks: %d", usedBanks);
	ImGui::Text("Total pages: %d", state.GetNoPages());
	ImGui::Text("Pages in use: %d", pagesInUse);

	ImGui::SeparatorText("Items");
	ImGui::Text("Itemlist size: %d", state.ItemList.size());
	ImGui::Text("Global data items size: %d", state.GlobalDataItems.size());
	ImGui::Text("Global functions items size: %d", state.GlobalFunctions.size());
}
