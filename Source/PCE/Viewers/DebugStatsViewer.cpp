#include "DebugStatsViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include "../GameDb.h"
#include "../DebugStats.h"
#include "BatchGameLoadViewer.h"

#include <geargrafx_core.h>

// todo knock this out in release build?

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

std::string GetBankType(Memory* pMemory, int bankIndex)
{
	switch (pMemory->GetBankType(bankIndex))
	{
	case Memory::MEMORY_BANK_TYPE_ROM:
		return "ROM";
	case Memory::MEMORY_BANK_TYPE_BIOS:
		return "BIOS";
	case Memory::MEMORY_BANK_TYPE_CARD_RAM:
		return "CARD RAM";
	case Memory::MEMORY_BANK_TYPE_BACKUP_RAM:
		return "BACKUP RAM";
	case Memory::MEMORY_BANK_TYPE_WRAM:
		return "WORK RAM";
	case Memory::MEMORY_BANK_TYPE_CDROM_RAM:
		return "CD ROM RAM";
	case Memory::MEMORY_BANK_TYPE_UNUSED:
		return "UNUSED RAM";
	}
	return "UNKNOWN";
}

enum EGameStatsColumns
{
	Col_GS_GameName,
	Col_GS_NumBanks,
	Col_GS_BanksMapped,
	Col_GS_MaxBankSwitches,
	Col_GS_AvgFPS,
	Col_GS_NumNonCanonicalBanksWithLabels,
	Col_GS_Count
};

static void SortGameStatsTable(std::vector<std::pair<std::string, const FGameDebugStats*>>& entries, ImGuiTableSortSpecs* sortSpecs)
{
	if (!sortSpecs || sortSpecs->SpecsCount == 0)
		return;

	const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];

	std::sort(entries.begin(), entries.end(),
		[&](const auto& A, const auto& B)
		{
			int result = 0;
			switch (spec.ColumnIndex)
			{
			case Col_GS_GameName:
				result = A.first.compare(B.first);
				break;
			case Col_GS_NumBanks:
				result = A.second->NumBanks - B.second->NumBanks;
				break;
			case Col_GS_BanksMapped:
				result = A.second->NumBanksMapped - B.second->NumBanksMapped;
				break;
			case Col_GS_MaxBankSwitches:
				result = A.second->MaxBankSwitches - B.second->MaxBankSwitches;
				break;
			case Col_GS_AvgFPS:
				result = (A.second->AvgFrameRate > B.second->AvgFrameRate) ? 1 : (A.second->AvgFrameRate < B.second->AvgFrameRate) ? -1 : 0;
				break;
			case Col_GS_NumNonCanonicalBanksWithLabels:
				result = A.second->NumNonCanonicalBanksWithLabels - B.second->NumNonCanonicalBanksWithLabels;
				break;
			}
			if (spec.SortDirection == ImGuiSortDirection_Descending)
				result = -result;
			return result < 0;
		});
}

void FDebugStatsViewer::DrawUI()
{
	if (!pPCEEmu->pDebugStats)
		return;

	if (ImGui::BeginTabBar("debugstats_tab_bar"))
	{
		if (ImGui::BeginTabItem("General"))
		{
			DrawGeneralStats();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Table"))
		{
			DrawDebugStatsTable();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Bank List"))
		{
			DrawBankList();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Bank Sets"))
		{
			DrawBankSets();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void FDebugStatsViewer::DrawBankSets()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	ImGui::Checkbox("Only show problem labels", &bOnlyShowProblemLabels);

	for (int i = 0; i < FPCEEmu::kNumBanks; i++)
	{
		const FBankSet& bankSet = pPCEEmu->GetBankSet(i);
		if (bankSet.Banks.empty())
			continue;

		if (!bOnlyShowProblemLabels)
		{
			char tmp[32];
			sprintf(tmp, "%d", i);
			ImGui::SeparatorText(tmp);
		}

		const int16_t canonicalBankId = bankSet.GetBankId();
		for (auto& entry : bankSet.Banks)
		{
			FCodeAnalysisBank* pBank = state.GetBank(entry.BankId);
			int numLabels = 0;
			int numCodeItems = 0;
			if (pBank)
			{
#ifndef NDEBUG
				// todo knock this out in a better way
				numLabels = pBank->NumLabels;
				numCodeItems = pBank->NumCodeItems;
#endif
			}
			bool bShowLabel = !bOnlyShowProblemLabels;
			const bool bIsCanonical = entry.BankId == canonicalBankId;
			const bool bIsBankOfInterest = !bIsCanonical || i == 136 /*UNUSED*/;
			if (bOnlyShowProblemLabels && bIsBankOfInterest && numLabels)
				bShowLabel = true;
			if (bShowLabel)
			{
				ImGui::Text("  %10s: %d labels %d code items", pBank ? pBank->Name.c_str() : "none", numLabels, numCodeItems);
			}
		}
	}
}

void FDebugStatsViewer::DrawDebugStatsTable()
{
	if (ImGui::Button("Reset"))
		pPCEEmu->pDebugStats->Reset();
	ImGui::SameLine();
	if (ImGui::Button("Dump"))
		bDumpBanks = true;

	static std::vector<std::pair<std::string, const FGameDebugStats*>> SortedGameStats;
	static size_t LastGameStatsSize = 0;

	const ImGuiTableFlags tableFlags =
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Sortable |
		ImGuiTableFlags_ScrollY;

	if (ImGui::BeginTable("GameStatsTable", Col_GS_Count, tableFlags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Game", ImGuiTableColumnFlags_DefaultSort);
		ImGui::TableSetupColumn("Num Banks");
		ImGui::TableSetupColumn("Banks Mapped");
		ImGui::TableSetupColumn("Max Bank Switches");
		ImGui::TableSetupColumn("Avg FPS");
		ImGui::TableSetupColumn("Num Problem Banks With Labels");
		ImGui::TableHeadersRow();

		const auto& gameDebugStats = pPCEEmu->pDebugStats->GameDebugStats;

		if (gameDebugStats.size() != LastGameStatsSize)
		{
			LastGameStatsSize = gameDebugStats.size();
			SortedGameStats.clear();
			SortedGameStats.reserve(gameDebugStats.size());
			for (const auto& pair : gameDebugStats)
				SortedGameStats.push_back({ pair.first, &pair.second });
		}

		ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
		if (sortSpecs && sortSpecs->SpecsDirty)
		{
			SortGameStatsTable(SortedGameStats, sortSpecs);
			sortSpecs->SpecsDirty = false;
		}

		for (const auto& entry : SortedGameStats)
		{
			const std::string& gameName = entry.first;
			const FGameDebugStats& gameStats = *entry.second;
			const bool bAllMapped = gameStats.NumBanksMapped == gameStats.NumBanks;

			ImGui::TableNextRow();

			if (bAllMapped)
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 0.5f, 0.f, 1.0f)));

			ImGui::TableSetColumnIndex(Col_GS_GameName);
			ImGui::TextUnformatted(gameName.c_str());

			ImGui::TableSetColumnIndex(Col_GS_NumBanks);
			ImGui::Text("%d", gameStats.NumBanks);

			ImGui::TableSetColumnIndex(Col_GS_BanksMapped);
			ImGui::Text("%d/%d", gameStats.NumBanksMapped, gameStats.NumBanks);

			ImGui::TableSetColumnIndex(Col_GS_MaxBankSwitches);
			ImGui::Text("%d", gameStats.MaxBankSwitches);

			ImGui::TableSetColumnIndex(Col_GS_AvgFPS);
			ImGui::Text("%.1f", gameStats.AvgFrameRate);

			ImGui::TableSetColumnIndex(Col_GS_NumNonCanonicalBanksWithLabels);
			ImGui::Text("%d", gameStats.NumNonCanonicalBanksWithLabels);
		}

		ImGui::EndTable();
	}
}

void FDebugStatsViewer::DrawBankList()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	constexpr ImVec4 redColour(1.0f, 0.0f, 0.0f, 1.0f);
	constexpr ImVec4 whiteColour(1.0f, 1.0f, 1.0f, 1.0f);
	constexpr ImVec4 yellowColour(1.0f, 1.0f, 0.0f, 1.0f);
	constexpr ImVec4 greenColour(0.0f, 1.0f, 0.0f, 1.0f);

	for (int i = 0; i < 256; i++)
	{
		if (const FCodeAnalysisBank* pBank = state.GetBank(pPCEEmu->Banks[i]->GetBankId(0)))
		{
			const uint8_t* gearGfxMem = pPCEEmu->GetMemory()->GetMemoryMap()[i];

			int index = -1;
			const Memory::MemoryBankType bankType = pPCEEmu->GetMemory()->GetBankType(i);
			if (bankType == Memory::MEMORY_BANK_TYPE_ROM || bankType == Memory::MEMORY_BANK_TYPE_BIOS)
				index = pPCEEmu->GetMedia()->GetRomBankIndex(i);
			else if (bankType == Memory::MEMORY_BANK_TYPE_CARD_RAM)
				index = i - pPCEEmu->GetMemory()->GetCardRAMStart();

			const bool bRam = pPCEEmu->GetMemory()->GetMemoryMapWrite()[i];

			ImVec4 colour;
			if (gearGfxMem != pBank->Memory)
				colour = redColour;
			else if ((bRam && pBank->Mapping != EBankAccess::ReadWrite) || (!bRam && pBank->Mapping != EBankAccess::Read))
				colour = yellowColour;
			else
				colour = whiteColour;

			ImGui::TextColored(colour, "%02x '%s' %02d %s '%s' %s",
				i,
				GetBankType(pPCEEmu->GetMemory(), i).c_str(),
				index,
				bRam ? "RW" : "R",
				pBank->Name.c_str(),
				pBank->Mapping == EBankAccess::Read ? "R" : pBank->Mapping == EBankAccess::ReadWrite ? "RW" : "?");
		}
	}
}

void FDebugStatsViewer::DrawGeneralStats()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

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
	float lowestFPS = FLT_MAX;
	std::string gameWithLowestFPS;
	for (auto pair : pPCEEmu->pDebugStats->GameDebugStats)
	{
		const FGameDebugStats& gameStats = pair.second;
		if (gameStats.NumDupeBanks > maxDupeBanks)
		{
			maxDupeBanks = gameStats.NumDupeBanks;
			gameWithMaxDupes = pair.first;
		}
		if (gameStats.AvgFrameRate < lowestFPS)
		{
			gameWithLowestFPS = pair.first;
			lowestFPS = gameStats.AvgFrameRate;
		}
	}

	const int romSize = pPCEEmu->GetMedia()->IsCDROM() ? GG_BIOS_SYSCARD_SIZE : pPCEEmu->GetMedia()->GetROMSize();
	const int romBankCount = (romSize / 0x2000) + (romSize % 0x2000 ? 1 : 0);

	ImGui::SeparatorText("Banks");
	ImGui::Text("Total banks: %d", FCodeAnalysisState::BankCount);
	ImGui::Text("Mapped banks: %d", mappedBanks);
	ImGui::Text("Banks with primary mapped page: %d", banksWithPrimaryMappedPage);
	ImGui::Text("Used banks: %d", usedBanks);
	ImGui::Text("Total pages: %d", state.GetNoPages());
	ImGui::Text("Pages in use: %d", pagesInUse);
	ImGui::Text("Game with most dupe banks: %s (%d)", gameWithMaxDupes.c_str(), maxDupeBanks);
	ImGui::Text("Game with lowest FPS: %s (%.1f)", gameWithLowestFPS.c_str(), lowestFPS);
	ImGui::Text("Max dupe banks: %d", maxDupeBanks);
	ImGui::Text("Num bank sets: %d", kNumBankSetIds);
	ImGui::Text("Bank switches per frame: %d", pPCEEmu->pDebugStats->NumBankSwitchesThisFrame);

	constexpr ImVec4 redColour(1.0f, 0.0f, 0.0f, 1.0f);
	constexpr ImVec4 whiteColour(1.0f, 1.0f, 1.0f, 1.0f);
	constexpr ImVec4 yellowColour(1.0f, 1.0f, 0.0f, 1.0f);
	constexpr ImVec4 greenColour(0.0f, 1.0f, 0.0f, 1.0f);

	bDumpBanks = false;

	for (const auto& pair : pPCEEmu->pDebugStats->GameDebugStats)
	{
		const FGameDebugStats& gameStats = pair.second;

		if (bDumpBanks)
		{
			LOGINFO("%s", pair.first.c_str());
			LOGINFO("  Num Banks: %d", gameStats.NumBanks);
			LOGINFO("  Num Banks Mapped: %d/%d (%.2f%%)", gameStats.NumBanksMapped, gameStats.NumBanks, ((float)gameStats.NumBanksMapped / (float)gameStats.NumBanks) * 100.f);
			LOGINFO("  Avg FPS: %1.f", gameStats.AvgFrameRate);
		}

		if (gameStats.NumBanksMapped == gameStats.NumBanks)
		{
			std::map<std::string, float>::iterator it = TimeUntilMapped.find(pair.first);
			if (it == TimeUntilMapped.end())
				TimeUntilMapped[pair.first] = pPCEEmu->GetBatchGameLoadViewer()->GetElapsedGameRunTime();
		}
	}
	
	// dont think we need this now we have the banks view in game db?
	/*if (ImGui::TreeNode("Games Bank mappings"))
	{
		TGameDb& gameDb = GetGameDb();
		for (const auto it : gameDb)
		{
			const FGameDebugStats* pGameStats = pPCEEmu->pDebugStats->GetDebugStatsForGame(it.first);
			const FGameDbEntry& entry = it.second;
			bool bTreeOpen = ImGui::TreeNode(it.first.c_str());

			if (pGameStats)
			{
				ImGui::SameLine();
				if (pGameStats->NumBanksMapped == pGameStats->NumBanks)
				{
					if (entry.NumDynamicBanks == 0)
						ImGui::TextColored(greenColour, "FIXED");
					else
						ImGui::TextColored(yellowColour, "DYNAMIC");
				}
				else
					ImGui::TextColored(redColour, "INCOMPLETE");
			}

			if (bTreeOpen)
			{
				//ImGui::Text("%s", it.first.c_str());
				for (int i = 0; i < entry.Banks.size(); i++)
				{
					const int mprSlot = entry.Banks[i].MprSlots[0];
					if (mprSlot == -1)
						ImGui::Text("  %02d - ----", i);
					else
						ImGui::TextColored(!entry.Banks[i].bFixed ? yellowColour : whiteColour, "  %02d %d %04x", i, mprSlot, mprSlot * 0x2000);
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}*/

	if (ImGui::TreeNode("Games with all banks mapped"))
	{
		TGameDb& gameDb = GetGameDb();

		if (bDumpBanks)
		{
			LOGINFO("Mapped games");
		}

		for (auto pair : TimeUntilMapped)
		{
			const FGameDbEntry& entry = gameDb[pair.first];
			ImGui::TextColored(entry.NumDynamicBanks > 0 ? yellowColour : whiteColour, "%s %d [%.2f secs]", pair.first.c_str(), entry.NumDynamicBanks, pair.second);

			if (bDumpBanks)
			{
				LOGINFO("  %s %d [%.2f secs]", pair.first.c_str(), entry.NumDynamicBanks, pair.second);
			}
		}
		ImGui::TreePop();
	}

	ImGui::SeparatorText("Items");
	ImGui::Text("Itemlist size: %d", state.ItemList.size());
	ImGui::Text("Global data items size: %d", state.GlobalDataItems.size());
	ImGui::Text("Global functions items size: %d", state.GlobalFunctions.size());

	ImGui::SeparatorText("Rom/Bios");
	ImGui::Text("Type : %s", pPCEEmu->GetMedia()->IsCDROM() ? "BIOS" : "ROM");
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

	ImGui::SeparatorText("Save RAM");
	ImGui::Text("Enabled: %s", pPCEEmu->GetMemory()->IsBackupRamEnabled() ? "Yes" : "No");
	ImGui::Text("In Use: %s", pPCEEmu->GetMemory()->IsBackupRamUsed() ? "Yes" : "No");
}