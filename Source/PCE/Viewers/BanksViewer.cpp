#include "BanksViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "../GameDb.h"
#include <Misc/GameConfig.h>

enum class EBankTableColumn : int
{
	EverMapped = 0,
	Access,
	Name,
	Address,
	Content,
	MprSlots,
};

enum class EBankContent : int
{
	Data = 0,
	Code,
	Mixed,
	Unknown,
};

static const char* BankAccessToString(EBankAccess access)
{
	switch (access)
	{
	case EBankAccess::Read:       return "R";
	case EBankAccess::Write:      return "W";
	case EBankAccess::ReadWrite:  return "RW";
	default:                      return "-";
	}
}

static EBankContent GetBankContent(const FCodeAnalysisBank* pBank)
{
	if (pBank->bHasCode)
		return EBankContent::Code;
	if (pBank->bHasData)
		return EBankContent::Data;
	return EBankContent::Unknown;
}

static const char* BankContentToString(EBankContent content)
{
	switch (content)
	{
		case EBankContent::Data:		return "Data";
		case EBankContent::Code:		return "Code";
		case EBankContent::Mixed:		return "Mixed";

		case EBankContent::Unknown:
		default:								return "Unknown";
	}
}

static void SortBankTable(const ImGuiTableSortSpecs* sortSpecs,	const std::vector<FCodeAnalysisBank*>& Banks,	std::vector<int>& sortedIndices)
{
	if (!sortSpecs || sortSpecs->SpecsCount == 0)
		return;

	const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];

	auto Compare = [&](int lhs, int rhs)
		{
			const FCodeAnalysisBank* A = Banks[lhs];
			const FCodeAnalysisBank* B = Banks[rhs];
			int delta = 0;

			switch ((EBankTableColumn)spec.ColumnIndex)
			{
			case EBankTableColumn::Name:
				delta = A->Name.compare(B->Name);
				break;

			case EBankTableColumn::Access:
				delta = int(A->Mapping) - int(B->Mapping);
				break;

			case EBankTableColumn::Address:
				delta = int(A->GetMappedAddress()) - int(B->GetMappedAddress());
				break;

			case EBankTableColumn::EverMapped:
				delta = int(A->bEverBeenMapped) - int(B->bEverBeenMapped);
				break;
			}

			if (spec.SortDirection == ImGuiSortDirection_Descending)
				delta = -delta;

			return delta < 0;
		};

	std::sort(sortedIndices.begin(), sortedIndices.end(), Compare);
}

void FBanksViewer::DrawBankTable(const std::vector<FCodeAnalysisBank*>& Banks, const std::vector<FBankSet*>& BankSets)
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	FGameDbEntry* pDbEntry = nullptr;
	if (pPCEEmu->GetProjectConfig())
	{
		pDbEntry = GetGameDbEntry(pPCEEmu->GetProjectConfig()->Name);
	}

	static std::vector<int> sortedIndices;

	// Initialize index list once or if size changes
	if (sortedIndices.size() != Banks.size())
	{
		sortedIndices.resize(Banks.size());
		for (int i = 0; i < (int)Banks.size(); ++i)
			sortedIndices[i] = i;
		SelectedBankIdx = -1;
	}

	ImGuiTableFlags flags =
		ImGuiTableFlags_Borders |
		//ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_Reorderable |
		ImGuiTableFlags_Sortable |
		ImGuiTableFlags_SortMulti |
		ImGuiTableFlags_Hideable |
		ImGuiTableFlags_ScrollY;

	if (ImGui::BeginTable("MemoryBanksTable", 6, flags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);

		ImGui::TableSetupColumn("Mapping State", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed, 10.f, (int)EBankTableColumn::EverMapped);
		ImGui::TableSetupColumn("Access",	ImGuiTableColumnFlags_PreferSortDescending,	0.0f,	(int)EBankTableColumn::Access);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort, 0.0f, (int)EBankTableColumn::Name);
		ImGui::TableSetupColumn("Mapped Address",	ImGuiTableColumnFlags_PreferSortDescending,	0.0f,	(int)EBankTableColumn::Address);
		ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_PreferSortDescending, 0.0f, (int)EBankTableColumn::Content);
		ImGui::TableSetupColumn("MPR Slots", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, (int)EBankTableColumn::MprSlots);
		ImGui::TableHeadersRow();

		// Handle sorting
		if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
		{
			if (sortSpecs->SpecsDirty)
			{
				SortBankTable(sortSpecs, Banks, sortedIndices);
				sortSpecs->SpecsDirty = false;
			}
		}

		// Draw rows
		for (int idx : sortedIndices)
		{
			const FCodeAnalysisBank* pBank = Banks[idx];

			ImGui::TableNextRow();

			constexpr ImVec4 mappedColour(0.0f, 0.75f, 0.0f, 1.0f);
			constexpr ImVec4 previouslyMappedColour(1.0f, 1.0f, 1.0f, 1.0f);
			constexpr ImVec4 neverMappedColour(0.56f, 0.56f, 0.56f, 1.0f);
			//constexpr ImVec4 neverMappedColour(0.28f, 0.28f, 0.28f, 1.0f);

			ImVec4 colour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			const bool bUseColouredText = true;
			if (bUseColouredText)
			{
				colour = neverMappedColour;
				if (pBank->bEverBeenMapped)
				{
					//colour = pBank->Mapping != EBankAccess::None ? mappedColour : previouslyMappedColour;
					colour = previouslyMappedColour;
				}
			}

			// Mapping State
			ImGui::TableSetColumnIndex(0);

			const bool bSelected = (SelectedBankIdx == idx);
			ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick;
			ImGui::PushID(idx);
			if (ImGui::Selectable("##mappingstate", bSelected, selectable_flags))
			{
				SelectedBankIdx = idx;
				if (pBank->bEverBeenMapped)
				{
					const FAddressRef bankAddr(pBank->Id, pBank->GetMappedAddress());
					if (ImGui::IsMouseDoubleClicked(0))
						state.GetFocussedViewState().GoToAddress(bankAddr, false);
				}
			}
			ImGui::PopID();

			// Colour cell to show mapping state
			{
				ImVec4 cellBgColour = neverMappedColour;
				if (pBank->bEverBeenMapped)
				{
					cellBgColour = pBank->Mapping != EBankAccess::None ? mappedColour : previouslyMappedColour;
				}
				ImU32 imU32CellBgColour = ImGui::ColorConvertFloat4ToU32(cellBgColour);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, imU32CellBgColour);
			}

			// Access
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(colour, BankAccessToString(pBank->Mapping));

			// Name
			ImGui::TableSetColumnIndex(2);
			ImGui::TextColored(colour, pBank->Name.c_str());
			
			// Mapped address
			ImGui::TableSetColumnIndex(3);
			if (pBank->bEverBeenMapped)
			{
				ImGui::TextColored(colour, "%s", NumStr(pBank->GetMappedAddress()));
				if (ImGui::IsItemHovered())
				{
					const FAddressRef bankAddr(pBank->Id, pBank->GetMappedAddress());
					DrawSnippetToolTip(state, state.GetFocussedViewState(), bankAddr, 11);
				}
			}
			else if (pDbEntry && idx < pDbEntry->Banks.size() && !pDbEntry->Banks[idx].MprSlots.empty())
			{
				ImGui::TextColored(colour, "%s", NumStr(pDbEntry->Banks[idx].GetMappedAddress()));
			}
			else
			{
				ImGui::TextColored(colour, "----");
			}

			// Content
			// todo figure out why this doesn't work if the code analysis view is not active
			ImGui::TableSetColumnIndex(4);
			ImGui::TextColored(colour, pBank->bEverBeenMapped ? BankContentToString(GetBankContent(pBank)) : "-");

			// MPR Slots
			ImGui::TableSetColumnIndex(5);
			{
				const FBankSet* pBankSet = BankSets[idx];
				const float squareSize = ImGui::GetTextLineHeight() - 2.0f;
				const float gap = 2.0f;
				const ImVec2 startPos = ImGui::GetCursorScreenPos();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				for (int slot = 0; slot < 8; slot++)
				{
					const float x = startPos.x + slot * (squareSize + gap);
					const float y = startPos.y + 1.0f;
					const ImVec2 pMin(x, y);
					const ImVec2 pMax(x + squareSize, y + squareSize);

					if (pBankSet->SlotBankId[slot] != -1)
						pDrawList->AddRectFilled(pMin, pMax, IM_COL32(0, 192, 0, 255));
					else if (pBankSet->MappedSlotsMask & (1 << slot))
						pDrawList->AddRectFilled(pMin, pMax, IM_COL32(255, 255, 255, 255));
					else
						pDrawList->AddRect(pMin, pMax, IM_COL32(160, 160, 160, 255));

					if (ImGui::IsMouseHoveringRect(pMin, pMax))
						ImGui::SetTooltip("MPR %d : %s", slot, NumStr((uint16_t)(slot * 0x2000)));
				}

				ImGui::Dummy(ImVec2(8.0f * squareSize + 7.0f * gap, squareSize));
			}
		}

		ImGui::EndTable();
	}
}

void FBanksViewer::DrawBankDetail(const FBankSet* pBankSet, const FCodeAnalysisBank* pBank)
{
	ImGui::Text("Bank: %s", pBank->Name.c_str());
	ImGui::Separator();

	// MPR slot map count table
	ImGui::Text("Times mapped per MPR slot:");
	ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable("MPRSlotTable", 8, flags))
	{
		for (int slot = 0; slot < 8; slot++)
		{
			char label[4];
			snprintf(label, sizeof(label), "%d", slot);
			ImGui::TableSetupColumn(label);
		}
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();
		for (int slot = 0; slot < 8; slot++)
		{
			ImGui::TableSetColumnIndex(slot);
			const uint32_t count = pBankSet->SlotMapCount[slot];
			if (count > 0)
				ImGui::Text("%u", count);
			else
				ImGui::TextDisabled("-");
		}

		ImGui::EndTable();
	}

	// First-use mapping order
	ImGui::Spacing();
	ImGui::Text("First mapped order:");

	// Collect slots sorted by first-use order
	int orderToSlot[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 }; // index 1..8
	for (int slot = 0; slot < 8; slot++)
	{
		const uint8_t order = pBankSet->SlotFirstUseOrder[slot];
		if (order > 0 && order <= 8)
			orderToSlot[order] = slot;
	}

	bool bAny = false;
	for (int order = 1; order <= 8; order++)
	{
		if (orderToSlot[order] != -1)
		{
			if (bAny)
				ImGui::SameLine();
			ImGui::Text("MPR%d", orderToSlot[order]);
			bAny = true;
		}
	}
	ImGui::Text("Estimated Mapped Address: %s", NumStr(pBankSet->GetMappedAddressFromUsage()));
	if (!bAny)
		ImGui::TextDisabled("(never mapped)");
}

FBanksViewer::FBanksViewer(FEmuBase* pEmu)
: FViewerBase(pEmu)
{
	Name = "Banks";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FBanksViewer::Init()
{
	return true;
}

void FBanksViewer::DrawMappedBanks()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	std::vector<FCodeAnalysisBank*> banksToView;
	std::vector<FBankSet*> bankSetsToView;

	for (int addr = 0; addr < 0x10000; addr += 0x2000)
	{
		const int bankId = state.GetBankFromAddress(addr);
		FCodeAnalysisBank* pBank = state.GetBank(bankId);
		if (!pBank)
			continue;

		FBankSet* pBankSet = pPCEEmu->GetBankSetFromBankId(bankId);
		if (!pBankSet)
			continue;

		// Avoid duplicates (same bank mapped to multiple slots)
		/*if (std::find(banksToView.begin(), banksToView.end(), pBank) == banksToView.end())
		{
			banksToView.push_back(pBank);
			bankSetsToView.push_back(pBankSet);
		}*/
	}

	const float detailWidth = ImGui::GetFontSize() * 28.0f;
	const float tableWidth = ImGui::GetContentRegionAvail().x - detailWidth - ImGui::GetStyle().ItemSpacing.x;

	// todo default to sorting by mapped address
	ImGui::BeginChild("##MappedBankTablePane", ImVec2(tableWidth, 0.0f), false);
	DrawBankTable(banksToView, bankSetsToView);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("##MappedBankDetailPane", ImVec2(detailWidth, 0.0f), false);
	if (SelectedBankIdx >= 0 && SelectedBankIdx < (int)banksToView.size())
		DrawBankDetail(bankSetsToView[SelectedBankIdx], banksToView[SelectedBankIdx]);
	else
		ImGui::TextDisabled("Select a bank to see details.");
	ImGui::EndChild();
}

void FBanksViewer::DrawAllBanks()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	std::vector<FCodeAnalysisBank*> banksToView;
	std::vector<FBankSet*> bankSetsToView;

	for (int i = 0; i < 0x80; i++)
	{
		FBankSet* pBankSet = pPCEEmu->Banks[i];
		const int16_t bankId = pBankSet->GetBankId(0);
		if (FCodeAnalysisBank* pBank = state.GetBank(bankId))
		{
			if (std::find(banksToView.begin(), banksToView.end(), pBank) == banksToView.end())
			{
				banksToView.push_back(pBank);
				bankSetsToView.push_back(pBankSet);
			}
		}
	}

	// WRAM
	{
		FBankSet* pBankSet = pPCEEmu->Banks[0xf8];
		const int16_t ramBankId = pBankSet->GetBankId(0);
		if (FCodeAnalysisBank* pBank = state.GetBank(ramBankId))
		{
			banksToView.push_back(pBank);
			bankSetsToView.push_back(pBankSet);
		}
	}

	const float detailWidth = ImGui::GetFontSize() * 20.0f;
	const float tableWidth = ImGui::GetContentRegionAvail().x - detailWidth - ImGui::GetStyle().ItemSpacing.x;

	ImGui::BeginChild("##BankTablePane", ImVec2(tableWidth, 0.0f), false);
	DrawBankTable(banksToView, bankSetsToView);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("##BankDetailPane", ImVec2(detailWidth, 0.0f), false);
	if (SelectedBankIdx >= 0 && SelectedBankIdx < (int)banksToView.size())
		DrawBankDetail(bankSetsToView[SelectedBankIdx], banksToView[SelectedBankIdx]);
	else
		ImGui::TextDisabled("Select a bank to see details.");
	ImGui::EndChild();
}

void FBanksViewer::DrawUI()
{
	if (ImGui::BeginTabBar("BanksTabBar"))
	{
		if (ImGui::BeginTabItem("All"))
		{
			DrawAllBanks();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Mapped"))
		{
			DrawMappedBanks();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}
