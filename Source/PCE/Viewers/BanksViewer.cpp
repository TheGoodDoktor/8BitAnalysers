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

void FBanksViewer::DrawBankTable(const std::vector<FCodeAnalysisBank*>& Banks)
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

	if (ImGui::BeginTable("MemoryBanksTable", 5, flags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);

		ImGui::TableSetupColumn("Mapping State", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed, 10.f, (int)EBankTableColumn::EverMapped);
		ImGui::TableSetupColumn("Access",	ImGuiTableColumnFlags_PreferSortDescending,	0.0f,	(int)EBankTableColumn::Access);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort, 0.0f, (int)EBankTableColumn::Name);
		ImGui::TableSetupColumn("Mapped Address",	ImGuiTableColumnFlags_PreferSortDescending,	0.0f,	(int)EBankTableColumn::Address);
		ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_PreferSortDescending, 0.0f, (int)EBankTableColumn::Content);
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

			ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick;
			if (ImGui::Selectable("##mappingstate", false, selectable_flags))
			{
				if (pBank->bEverBeenMapped)
				{
					const FAddressRef bankAddr(pBank->Id, pBank->GetMappedAddress());
					if (ImGui::IsMouseDoubleClicked(0))
						state.GetFocussedViewState().GoToAddress(bankAddr, false);
				}
			}

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
		}

		ImGui::EndTable();
	}
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

void FBanksViewer::DrawUI()
{
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();

	std::vector<FCodeAnalysisBank*> banksToView;
	//if (!pMedia->IsCDROM())


	for (int i = 0; i < 0x80; i++)
	{
		const int16_t bankId = pPCEEmu->Banks[i]->GetBankId(0);
		if (FCodeAnalysisBank* pBank = state.GetBank(bankId))
		{
			if (std::find(banksToView.begin(), banksToView.end(), pBank) == banksToView.end())
			{
				banksToView.push_back(pBank);
			}
		}
	}

	// WRAM
	const int16_t ramBankId = pPCEEmu->Banks[0xf8]->GetBankId(0);
	if (FCodeAnalysisBank* pBank = state.GetBank(ramBankId))
	{
		banksToView.push_back(pBank);
	}

	DrawBankTable(banksToView);
}
