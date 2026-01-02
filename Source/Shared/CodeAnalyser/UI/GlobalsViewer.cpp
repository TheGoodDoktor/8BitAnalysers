#include "GlobalsViewer.h"

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "CodeAnalyserUI.h"
#include "../CodeAnalyser.h"
#include "Misc/EmuBase.h"


enum EGlobalsColumnID
{
	// Used by code and data items
	Name = 0,
	References,
	Location,

	// Used by code items
	CallFrequencyIndicator,
	CallFrequencyCount,

	// Used by data items
	ReadIndicator,
	WriteIndicator,
	ReadCount,
	WriteCount,

	Count,
};

bool gbGlobalsColumnSortAlways[EGlobalsColumnID::Count] =
{
	false,
	false,
	false,

	true,
	true,

	true,
	true,
	true,
	true,
};


void SortGlobals(FCodeAnalysisState& state, std::vector<FCodeAnalysisItem>& globals, const ImGuiTableSortSpecs* pSortSpecs)
{
	if (pSortSpecs->SpecsCount != 1)
		return;

	const ImGuiTableColumnSortSpecs* pColumnSortSpecs = &pSortSpecs->Specs[0];
	switch (pColumnSortSpecs->ColumnUserID)
	{
	case EGlobalsColumnID::Name:
		std::sort(globals.begin(), globals.end(), [&state, &pColumnSortSpecs](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
			{
				const FLabelInfo* pLabelA = state.GetLabelForAddress(a.AddressRef);
				const FLabelInfo* pLabelB = state.GetLabelForAddress(b.AddressRef);

				std::string labelALower = pLabelA->GetName();
				std::transform(labelALower.begin(), labelALower.end(), labelALower.begin(), [](unsigned char c) { return std::tolower(c); });
				std::string labelBLower = pLabelB->GetName();
				std::transform(labelBLower.begin(), labelBLower.end(), labelBLower.begin(), [](unsigned char c) { return std::tolower(c); });

				if (pColumnSortSpecs->SortDirection == ImGuiSortDirection_Descending)
					return labelALower < labelBLower;	// dodgy!
				else
					return labelALower > labelBLower;	// dodgy!
			});
		break;
	case EGlobalsColumnID::References:
		std::sort(globals.begin(), globals.end(), [&state, &pColumnSortSpecs](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
			{
				const FLabelInfo* pLabelA = state.GetLabelForAddress(a.AddressRef);
				const FLabelInfo* pLabelB = state.GetLabelForAddress(b.AddressRef);
				if (pColumnSortSpecs->SortDirection == ImGuiSortDirection_Descending)
					return pLabelA->References.NumReferences() > pLabelB->References.NumReferences();
				else
					return pLabelA->References.NumReferences() < pLabelB->References.NumReferences();
			});
		break;
	case EGlobalsColumnID::Location:
		std::sort(globals.begin(), globals.end(), [&state, &pColumnSortSpecs](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
			{
				if (pColumnSortSpecs->SortDirection == ImGuiSortDirection_Descending)
					return a.AddressRef.GetAddress() > b.AddressRef.GetAddress();
				else
					return a.AddressRef.GetAddress() < b.AddressRef.GetAddress();
			});
		break;
	case EGlobalsColumnID::CallFrequencyIndicator:
	case EGlobalsColumnID::CallFrequencyCount:
		std::sort(globals.begin(), globals.end(), [&state, &pColumnSortSpecs](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
			{
				const FCodeInfo* pCodeInfoA = state.GetCodeInfoForAddress(a.AddressRef);
				const FCodeInfo* pCodeInfoB = state.GetCodeInfoForAddress(b.AddressRef);

				const int countA = pCodeInfoA != nullptr ? pCodeInfoA->ExecutionCount : 0;
				const int countB = pCodeInfoB != nullptr ? pCodeInfoB->ExecutionCount : 0;

				if (pColumnSortSpecs->SortDirection == ImGuiSortDirection_Descending)
					return countA > countB;
				else
					return countA < countB;
			});
		break;
	case EGlobalsColumnID::ReadCount:
	case EGlobalsColumnID::ReadIndicator:
		std::sort(globals.begin(), globals.end(), [&state, &pColumnSortSpecs](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
			{
				const FDataInfo* pDataInfoA = state.GetDataInfoForAddress(a.AddressRef);
				const FDataInfo* pDataInfoB = state.GetDataInfoForAddress(b.AddressRef);

				if (pColumnSortSpecs->SortDirection == ImGuiSortDirection_Descending)
					return pDataInfoA->ReadCount > pDataInfoB->ReadCount;
				else
					return pDataInfoA->ReadCount < pDataInfoB->ReadCount;
			});
		break;
	case EGlobalsColumnID::WriteCount:
	case EGlobalsColumnID::WriteIndicator:
		std::sort(globals.begin(), globals.end(), [&state, &pColumnSortSpecs](const FCodeAnalysisItem& a, const FCodeAnalysisItem& b)
			{
				const FDataInfo* pDataInfoA = state.GetDataInfoForAddress(a.AddressRef);
				const FDataInfo* pDataInfoB = state.GetDataInfoForAddress(b.AddressRef);

				if (pColumnSortSpecs->SortDirection == ImGuiSortDirection_Descending)
					return pDataInfoA->WriteCount > pDataInfoB->WriteCount;
				else
					return pDataInfoA->WriteCount < pDataInfoB->WriteCount;
			});
		break;
	}
}


void GenerateFilteredLabelList(FCodeAnalysisState& state, const FLabelListFilter& filter, const std::vector<FCodeAnalysisItem>& sourceLabelList, std::vector<FCodeAnalysisItem>& filteredList)
{
	filteredList.clear();

	std::string filterTextLower = filter.FilterText;
	std::transform(filterTextLower.begin(), filterTextLower.end(), filterTextLower.begin(), [](unsigned char c) { return std::tolower(c); });

	for (const FCodeAnalysisItem& labelItem : sourceLabelList)
	{
		if (labelItem.AddressRef.GetAddress() < filter.MinAddress || labelItem.AddressRef.GetAddress() > filter.MaxAddress)	// skip min address
			continue;

		const FCodeAnalysisBank* pBank = state.GetBank(labelItem.AddressRef.GetBankId());
		if (pBank)
		{
			if (filter.bNoMachineRoms && pBank->bMachineROM)
				continue;
		}

		if (filter.DataType != EDataTypeFilter::All)
		{
			if (const FDataInfo* pDataInfo = state.GetDataInfoForAddress(labelItem.AddressRef))
			{
				switch (filter.DataType)
				{
				case EDataTypeFilter::Pointer:
					if (pDataInfo->DisplayType != EDataItemDisplayType::Pointer)
						continue;
					break;
				case EDataTypeFilter::Text:
					if (pDataInfo->DataType != EDataType::Text)
						continue;
					break;
				case EDataTypeFilter::Bitmap:
					if (pDataInfo->DataType != EDataType::Bitmap)
						continue;
					break;
				case EDataTypeFilter::CharacterMap:
					if (pDataInfo->DataType != EDataType::CharacterMap)
						continue;
					break;
				case EDataTypeFilter::ColAttr:
					if (pDataInfo->DataType != EDataType::ColAttr)
						continue;
					break;
				default:
					break;
				}
			}
		}

		const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(labelItem.Item);
		std::string labelTextLower = pLabelInfo->GetName();
		std::transform(labelTextLower.begin(), labelTextLower.end(), labelTextLower.begin(), [](unsigned char c) { return std::tolower(c); });

		if (filter.FilterText.empty() || labelTextLower.find(filterTextLower) != std::string::npos)
			filteredList.push_back(labelItem);
	}
}

void DrawFunctionList(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, std::vector<FCodeAnalysisItem>& labelList, bool bSortNow)
{
	if (ImGui::BeginChild("GlobalFunctionList", ImVec2(0, 0), false))
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
#ifndef _NDEBUG
		flags |= ImGuiTableFlags_NoSavedSettings;
#endif
		const ImVec2 outer_size = ImVec2(0.0f, 0.0f);
		if (ImGui::BeginTable("GlobalFunctionTable", 5, flags, outer_size))
		{
			const int columnFlagsAsc = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortAscending;
			const int columnFlagsDes = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending;
			const int indicatorColumnFlags = columnFlagsDes | ImGuiTableColumnFlags_NoResize;// | ImGuiTableColumnFlags_NoHeaderLabel;
			const float w = ImGui_GetFontCharWidth();
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Ex", indicatorColumnFlags, w * 4.0f, EGlobalsColumnID::CallFrequencyIndicator);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, w * 32.0f, EGlobalsColumnID::Name);
			ImGui::TableSetupColumn("Refs", columnFlagsDes, w * 5.0f, EGlobalsColumnID::References);
			ImGui::TableSetupColumn("Addr", columnFlagsAsc | ImGuiTableColumnFlags_DefaultSort, w * 6.0f, EGlobalsColumnID::Location);
			ImGui::TableSetupColumn("Calls", columnFlagsDes | ImGuiTableColumnFlags_DefaultHide, w * 8.0f, EGlobalsColumnID::CallFrequencyCount);
			ImGui::TableHeadersRow();

			if (ImGuiTableSortSpecs* pSortSpecs = ImGui::TableGetSortSpecs())
			{
				if (pSortSpecs->SpecsCount == 1)
				{
					const ImGuiID columnID = pSortSpecs->Specs[0].ColumnUserID;
					bool bSort = bSortNow || gbGlobalsColumnSortAlways[columnID];

					if (pSortSpecs->SpecsDirty)
					{
						bSort = true;
						pSortSpecs->SpecsDirty = false;
					}
					if (bSort)
					{
						SortGlobals(state, labelList, pSortSpecs);
					}
				}
			}

			ImGuiListClipper clipper;
			clipper.Begin((int)labelList.size());

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					const FCodeAnalysisItem& item = labelList[i];
					const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(item.AddressRef);
					if (pCodeInfo && pCodeInfo->bDisabled == false)
						ShowCodeAccessorActivity(state, item.AddressRef);
					ImGui::TableNextColumn();
					
					const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(item.Item);

					// where should this go?
					ImGui::PushID(item.AddressRef.GetVal());

					if (ImGui::Selectable("##labellistitem", viewState.GetCursorItem().Item == pLabelInfo))
					{
						viewState.GoToAddress(item.AddressRef, true);
					}
					ImGui::SameLine();
					ImGui::Text("%s", pLabelInfo->GetName());
					ImGui::PopID();

					if (ImGui::IsItemHovered())
					{
						DrawSnippetToolTip(state, viewState, item.AddressRef);
					}

					ImGui::TableNextColumn();
					ImGui::Text("%d", pLabelInfo->References.NumReferences());

					ImGui::TableNextColumn();
					ImGui::Text("%s", NumStr(item.AddressRef.GetAddress()));

					ImGui::TableNextColumn();
					const int count = pCodeInfo != nullptr ? pCodeInfo->ExecutionCount : 0;
					ImGui::Text("%d", count);
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
}

void DrawGlobalDataList(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, std::vector<FCodeAnalysisItem>& labelList, bool bSortNow)
{
	if (ImGui::BeginChild("GlobalDataList", ImVec2(0, 0), false))
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
#ifndef _NDEBUG
		flags |= ImGuiTableFlags_NoSavedSettings;
#endif
		const ImVec2 outer_size = ImVec2(0.0f, 0.0f);
		if (ImGui::BeginTable("GlobalDataTable", 7, flags, outer_size))
		{
			const int columnFlagsAsc = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortAscending;
			const int columnFlagsDes = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending;
			const int indicatorColumnFlags = columnFlagsDes | ImGuiTableColumnFlags_NoResize;// | ImGuiTableColumnFlags_NoHeaderLabel;
			const float w = ImGui_GetFontCharWidth();
			const float rwIndicatorColWidth = w * 2.5f;
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("W", indicatorColumnFlags, rwIndicatorColWidth, EGlobalsColumnID::WriteIndicator);
			ImGui::TableSetupColumn("R", indicatorColumnFlags, rwIndicatorColWidth, EGlobalsColumnID::ReadIndicator);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, w * 32.0f, EGlobalsColumnID::Name);
			ImGui::TableSetupColumn("Refs", columnFlagsDes, w * 5.0f, EGlobalsColumnID::References);
			ImGui::TableSetupColumn("Addr", columnFlagsAsc | ImGuiTableColumnFlags_DefaultSort, w * 6.0f, EGlobalsColumnID::Location);
			ImGui::TableSetupColumn("R.Count", columnFlagsDes | ImGuiTableColumnFlags_DefaultHide, w * 8.0f, EGlobalsColumnID::ReadCount);
			ImGui::TableSetupColumn("W.Count", columnFlagsDes | ImGuiTableColumnFlags_DefaultHide, w * 8.0f, EGlobalsColumnID::WriteCount);
			ImGui::TableHeadersRow();

			if (ImGuiTableSortSpecs* pSortSpecs = ImGui::TableGetSortSpecs())
			{
				if (pSortSpecs->SpecsCount == 1)
				{
					const ImGuiID columnID = pSortSpecs->Specs[0].ColumnUserID;
					bool bSort = bSortNow || gbGlobalsColumnSortAlways[columnID];

					if (pSortSpecs->SpecsDirty)
					{
						bSort = true;
						pSortSpecs->SpecsDirty = false;
					}
					if (bSort)
					{
						SortGlobals(state, labelList, pSortSpecs);
					}
				}
			}

			ImGuiListClipper clipper;
			clipper.Begin((int)labelList.size());

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					const FCodeAnalysisItem& item = labelList[i];
					const FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(item.AddressRef);
					
					ShowDataItemWriteActivity(state, item.AddressRef);
					ImGui::TableNextColumn();

					ShowDataItemReadActivity(state, item.AddressRef);
					ImGui::TableNextColumn();
					const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(item.Item);

					// where should this go?
					ImGui::PushID(item.AddressRef.GetVal());

					if (ImGui::Selectable("##labellistitem", viewState.GetCursorItem().Item == pLabelInfo))
					{
						viewState.GoToAddress(item.AddressRef, true);
					}
					ImGui::SameLine();
					ImGui::Text("%s", pLabelInfo->GetName());
					ImGui::PopID();

					if (ImGui::IsItemHovered())
					{
						DrawSnippetToolTip(state, viewState, item.AddressRef);
					}

					ImGui::TableNextColumn();
					ImGui::Text("%d", pLabelInfo->References.NumReferences());

					ImGui::TableNextColumn();
					ImGui::Text("%s", NumStr(item.AddressRef.GetAddress()));

					
					const FDataInfo* pDataInfo = state.GetDataInfoForAddress(item.AddressRef);
					ImGui::TableNextColumn();
					ImGui::Text("%d", pDataInfo->ReadCount);

					ImGui::TableNextColumn();
					ImGui::Text("%d", pDataInfo->WriteCount);
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
}



bool	FGlobalsViewer::Init(void)
{
	return true;
}
void	FGlobalsViewer::Shutdown(void)
{

}


void FGlobalsViewer::DrawGlobals()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::InputText("Filter", &FilterText))
	{
		GlobalFunctionsFilter.FilterText = FilterText;
		GlobalDataItemsFilter.FilterText = FilterText;
		bRebuildFilteredGlobalFunctions = true;
		bRebuildFilteredGlobalDataItems = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("BIOS", &ShowROMLabels))
	{
		GlobalFunctionsFilter.bNoMachineRoms = !ShowROMLabels;
		GlobalDataItemsFilter.bNoMachineRoms = !ShowROMLabels;
		bRebuildFilteredGlobalFunctions = true;
		bRebuildFilteredGlobalDataItems = true;
	}

	if (ImGui::BeginTabBar("GlobalsTabBar"))
	{
		if (ImGui::BeginTabItem("Functions"))
		{
			bool bSort = false;
			if (bRebuildFilteredGlobalFunctions)
			{
				GenerateFilteredLabelList(state, GlobalFunctionsFilter, state.GlobalFunctions, FilteredGlobalFunctions);
				bSort = true;
				bRebuildFilteredGlobalFunctions = false;
			}

			DrawFunctionList(state, viewState, FilteredGlobalFunctions, bSort);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Data"))
		{
			bool bSort = false;
			if (bRebuildFilteredGlobalDataItems)
			{
				GenerateFilteredLabelList(state, GlobalDataItemsFilter, state.GlobalDataItems, FilteredGlobalDataItems);
				bRebuildFilteredGlobalDataItems = false;
				bSort = true;
			}

			DrawGlobalDataList(state, viewState, FilteredGlobalDataItems, bSort);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void	FGlobalsViewer::DrawUI() 
{
	DrawGlobals();
}

void FGlobalsViewer::FixupAddressRefs(void)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();

	for (FCodeAnalysisItem& item : FilteredGlobalDataItems)
		FixupAddressRef(state, item.AddressRef);
	for (FCodeAnalysisItem& item : FilteredGlobalFunctions)
		FixupAddressRef(state, item.AddressRef);
}
