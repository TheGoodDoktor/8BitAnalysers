#include "DebugStatsViewer.h"

//#include <algorithm>

#include <imgui.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>


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
	//Memory pMemory = pCE

	int mappedBanks = 0;
	int banksWithPrimaryMappedPage = 0;
	int usedBanks = 0;

	for (int b = 0; b < FCodeAnalysisState::BankCount; b++)
	{
		FCodeAnalysisBank& bank = state.GetBanks()[b];
	
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

	int maxDupeBanks = 0;
	std::string gameWithMaxDupes;
	for (auto pair : pPCEEmu->DebugStats.GamesWithDupeBanks)
	{
		if (pair.second > maxDupeBanks)
		{
			maxDupeBanks = pair.second;
			gameWithMaxDupes = pair.first;
		}
	}

	const int romSize = pPCEEmu->GetMedia()->GetROMSize();
	const int romBankCount = (romSize / 0x2000) + (romSize % 0x2000 ? 1 : 0);

	ImGui::SeparatorText("Banks");
	ImGui::Text("Total banks: %d", FCodeAnalysisState::BankCount);
	ImGui::Text("Mapped banks: %d", mappedBanks);
	ImGui::Text("Banks with primary mapped page: %d", banksWithPrimaryMappedPage);
	ImGui::Text("Used banks: %d", usedBanks);
	ImGui::Text("Total pages: %d", state.GetNoPages());
	ImGui::Text("Pages in use: %d", pagesInUse);
	ImGui::Text("Game with most dupe banks: %s", gameWithMaxDupes.c_str());
	ImGui::Text("Max dupe banks: %d", maxDupeBanks);
	ImGui::Text("Num bank sets: %d", pPCEEmu->kNumBankSetIds);

	ImGui::SeparatorText("Items");
	ImGui::Text("Itemlist size: %d", state.ItemList.size());
	ImGui::Text("Global data items size: %d", state.GlobalDataItems.size());
	ImGui::Text("Global functions items size: %d", state.GlobalFunctions.size());

	ImGui::SeparatorText("Rom");
	ImGui::Text("Size: %d", romSize);
	ImGui::Text("Bank count : %d", romBankCount);

	ImGui::SeparatorText("Debugger");
	ImGui::Text("Frame trace size: %d", state.Debugger.GetFrameTrace().size());
	ImGui::Text("Call stack size : %d", state.Debugger.GetCallstack().size());
	
	ImGui::SeparatorText("Analysis State");
	ImGui::Text("Current frame : %d", state.CurrentFrameNo);
	ImGui::Text("Execution counter : %d", state.ExecutionCounter);

	ImGui::SeparatorText("Video");
	ImGui::Text("6270 Hpos: %d", *pPCEEmu->GetCore()->GetHuC6270_1()->GetState()->HPOS);
	ImGui::Text("6270 Vpos: %d", pPCEEmu->GetVPos());
	//ImGui::Text("Raster line: %d", pPCEEmu->GetCore()->GetHuC6270_1()->m_raster_line);

	ImGui::SeparatorText("IO");
	const u8 ioReg = pPCEEmu->GetCore()->GetInput()->GetIORegister();
	ImGui::Text("Register: 0x%x %s", ioReg, NumStr(ioReg, ENumberDisplayMode::Binary));
}
