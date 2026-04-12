#include "GameDbViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include "../GameDb.h"
#include "../PCEConfig.h"

enum EGameDbColumns
{
	Col_GameName,
	Col_Overall,
	Col_AssemblesOk,
	Col_RomStatus,
	Col_TestMethodology,
	Col_MaxDupeBanks,
	Col_Count
};

bool GetOverallResult(const FGameDbEntry& entry)
{
	return entry.bAssemblesOk && entry.bRomFileIdentical;
}

const char* GetRomStatus(const FGameDbEntry& entry)
{
	if (entry.bRomFileIdentical)
		return "Identical";

	return "Fail";
}

static void SortGameDbTable(std::vector<std::pair<std::string, FGameDbEntry*>>& entries, ImGuiTableSortSpecs* sortSpecs)
{
	if (!sortSpecs || sortSpecs->SpecsCount == 0)
		return;

	const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];

	std::sort(entries.begin(), entries.end(),
		[&](const auto& A, const auto& B)
		{
			const std::string& nameA = A.first;
			const std::string& nameB = B.first;

			const FGameDbEntry& entryA = *A.second;
			const FGameDbEntry& entryB = *B.second;

			int result = 0;

			switch (spec.ColumnIndex)
			{
			case Col_GameName:
				result = nameA.compare(nameB);
				break;

			case Col_Overall:
				result = (int)GetOverallResult(entryA) - (int)GetOverallResult(entryB);
				break;

			case Col_AssemblesOk:
				result = (int)entryA.bAssemblesOk - (int)entryB.bAssemblesOk;
				break;

			case Col_RomStatus:
			{
				int a = entryA.bRomFileIdentical ? 2 : entryA.bRomFilePartialMatch ? 1 : 0;
				int b = entryB.bRomFileIdentical ? 2 : entryB.bRomFilePartialMatch ? 1 : 0;
				result = a - b;
				break;
			}
			case Col_MaxDupeBanks:
				result = entryA.MaxDupeBanks - entryB.MaxDupeBanks;
				break;


			case Col_TestMethodology:
				result = entryA.TestingMethodology - entryB.TestingMethodology;
				break;
			}

			if (spec.SortDirection == ImGuiSortDirection_Descending)
				result = -result;

			return result < 0;
		});
}

FGameDbViewer::FGameDbViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Game Db";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FGameDbViewer::Init()
{
	return true;
}

void FGameDbViewer::DrawUI()
{
	if (ImGui::BeginTabBar("gamedb_tab_bar"))
	{
		if (ImGui::BeginTabItem("Table"))
		{
			DrawGameDbTable();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Overview"))
		{
			DrawOverview();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Banks"))
		{
			DrawBanksTab();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void FGameDbViewer::DrawGameDbTable()
{
	if (ImGui::Button("Load All"))
	{
		auto findIt = pPCEEmu->GetGamesLists().find("Snapshot File");
		if (findIt != pPCEEmu->GetGamesLists().end())
		{
			const FGamesList& gamesList = findIt->second;
			for (int i = 0; i < gamesList.GetNoGames(); i++)
			{
				const FEmulatorFile& emuFile = gamesList.GetGame(i);
				const std::string fname = pPCEEmu->GetPCEGlobalConfig()->GameDbPath + emuFile.DisplayName + ".json";
				LoadGameDbEntry(emuFile.DisplayName, fname);
			}
		}
	}

	//FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	TGameDb& gameDb = GetGameDb();

	static std::vector<std::pair<std::string, FGameDbEntry*>> SortedEntries;
	static size_t LastDbSize = 0;

	ImGuiTableFlags flags =
		ImGuiTableFlags_Borders |
		//ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Sortable |
		ImGuiTableFlags_ScrollY;

	if (ImGui::BeginTable("GameDbTable", Col_Count, flags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);

		ImGui::TableSetupColumn("Game", ImGuiTableColumnFlags_DefaultSort);
		ImGui::TableSetupColumn("Overall");
		ImGui::TableSetupColumn("Assembles");
		ImGui::TableSetupColumn("ROM");
		ImGui::TableSetupColumn("Test Method");
		ImGui::TableSetupColumn("Max Dupe Banks");
		ImGui::TableHeadersRow();

		ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();

		bool rebuild = false;

		if (gameDb.size() != LastDbSize)
		{
			LastDbSize = gameDb.size();
			rebuild = true;
		}

		if (rebuild)
		{
			SortedEntries.clear();
			SortedEntries.reserve(gameDb.size());

			for (auto& it : gameDb)
				SortedEntries.push_back({ it.first, &it.second });
		}

		if (sortSpecs && sortSpecs->SpecsDirty)
		{
			SortGameDbTable(SortedEntries, sortSpecs);
			sortSpecs->SpecsDirty = false;
		}

		for (auto& pair : SortedEntries)
		{
			const std::string& gameName = pair.first;
			const FGameDbEntry& entry = *pair.second;

			bool overall = GetOverallResult(entry);

			ImGui::TableNextRow();

			// ----- Row colouring -----
			ImVec4 rowColor;

			const bool bValid = entry.bValidated;
			if (bValid)
			{
				if (overall)
					rowColor = ImVec4(0.f, 0.5f, 0.f, 1.0f);      // green
				else if (entry.bRomFilePartialMatch)
					rowColor = ImVec4(0.5f, 0.5f, 0.f, 1.0f);      // yellow
				else
					rowColor = ImVec4(0.5f, 0.0f, 0.0f, 1.0f);      // red

				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(rowColor));
			}

			// ----- Columns -----

			ImGui::TableSetColumnIndex(Col_GameName);
			ImGui::TextUnformatted(gameName.c_str());

			ImGui::TableSetColumnIndex(Col_Overall);
			ImGui::TextUnformatted(bValid ? overall ? "Pass" : "Fail" : "-");

			ImGui::TableSetColumnIndex(Col_AssemblesOk);
			ImGui::TextUnformatted(bValid ? entry.bAssemblesOk ? "Ok" : "Fail" : "-");

			ImGui::TableSetColumnIndex(Col_RomStatus);
			ImGui::TextUnformatted(bValid ? GetRomStatus(entry) : "-");

			ImGui::TableSetColumnIndex(Col_TestMethodology);

			if (entry.TestingMethodology == -1)
				ImGui::TextUnformatted("-");
			else
				ImGui::Text("%d", entry.TestingMethodology);

			ImGui::TableSetColumnIndex(Col_MaxDupeBanks);
			ImGui::Text("%d", entry.MaxDupeBanks);
		}

		ImGui::EndTable();
	}
}

void FGameDbViewer::DrawOverview()
{
	int numAssembles = 0;
	int numIdenticalRom = 0;
	int numPassOverall = 0;

	TGameDb& gameDb = GetGameDb();
	for (const auto& entry : gameDb)
	{
		if (!entry.second.bValidated)
			continue;

		if (entry.second.bAssemblesOk)
			numAssembles++;

		if (entry.second.bRomFileIdentical)
			numIdenticalRom++;
	}

	const int totNum = (int)gameDb.size();

	ImGui::Text("Assembles:     %3d / %3d", numAssembles, totNum);
	ImGui::Text("Identical ROM: %3d / %3d", numIdenticalRom, totNum);
}

void FGameDbViewer::DrawBanksTab()
{
	TGameDb& gameDb = GetGameDb();
	if (gameDb.empty())
	{
		ImGui::TextUnformatted("No game database entries loaded.");
		return;
	}

	// Selectable game list
	ImGui::BeginChild("##gamedb_banks_list", ImVec2(200.0f, 0), true);
	int idx = 0;
	for (auto& [gameName, entry] : gameDb)
	{
		if (ImGui::Selectable(gameName.c_str(), idx == m_BanksTabSelectedGame))
			m_BanksTabSelectedGame = idx;
		idx++;
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Table with bank details
	ImGui::BeginChild("##gamedb_banks_detail", ImVec2(0, 0), true);
	if (m_BanksTabSelectedGame != -1)
	{
		auto it = gameDb.begin();
		std::advance(it, m_BanksTabSelectedGame);
		const std::string& selectedGameName = it->first;
		FGameDbEntry* pEntry = &it->second;
		if (pEntry)
		{
			ImGui::Text("Banks: %s", selectedGameName.c_str());
			ImGui::Separator();

			const ImGuiTableFlags flags =
				ImGuiTableFlags_Borders |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_ScrollY |
				ImGuiTableFlags_Resizable;

			if (ImGui::BeginTable("##gamedb_bank_detail_table", 2, flags))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Bank Name");
				ImGui::TableSetupColumn("MPR Slots");
				ImGui::TableHeadersRow();

				for (int i = 0; i < (int)pEntry->Banks.size(); i++)
				{
					const FGameDbBank& dbBank = pEntry->Banks[i];

					// todo: deal with non rom banks
					std::string bankName = "ROM Bank " + std::to_string(i);

					std::string slotsStr;
					if (dbBank.MprSlots.empty())
					{
						slotsStr = "-";
					}
					else
					{
						for (int s = 0; s < (int)dbBank.MprSlots.size(); s++)
						{
							if (s > 0) slotsStr += " ";
							slotsStr += std::to_string(dbBank.MprSlots[s]);
						}
					}

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextUnformatted(bankName.c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::TextUnformatted(slotsStr.c_str());
				}

				ImGui::EndTable();
			}
		}
	}
	ImGui::EndChild();
}

