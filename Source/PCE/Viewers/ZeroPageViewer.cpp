#include "ZeroPageViewer.h"

#include <algorithm>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <vector>

#include "ImGuiSupport/ImGuiScaling.h"
#include "CodeAnalyser/CodeAnalyser.h"
#include "CodeAnalyser/UI/CodeAnalyserUI.h"
#include "CodeAnalyser/UI/UIColours.h"
#include "Util/Misc.h"
#include "../PCEEmu.h"
#include "../BankSet.h"


static constexpr uint16_t kZeroPageStart  = 0x2000;
static constexpr uint16_t kZeroPageEnd    = 0x20FF;
static constexpr uint8_t  kWRAM0BankIndex = 0xf8;

static void DrawZeroPageLabel(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	const FLabelInfo* pLabelInfo = static_cast<const FLabelInfo*>(item.Item);
	ImU32 colour = Colours::localLabel;
	if (viewState.HighlightAddress == item.AddressRef)
		colour = Colours::highlight;
	else if (pLabelInfo->LabelType == ELabelType::Function)
		colour = Colours::function;
	else if (pLabelInfo->Global)
		colour = Colours::globalLabel;
	ImGui::PushStyleColor(ImGuiCol_Text, colour);
	ImGui::Text("%s:", pLabelInfo->GetName());
	ImGui::PopStyleColor();
}

static void DrawZeroPageDataInfo(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	FDataInfo* pDataInfo = static_cast<FDataInfo*>(item.Item);
	const uint16_t physAddr = item.AddressRef.GetAddress();
	const bool bHighlight = viewState.HighlightAddress.IsValid() &&
		viewState.HighlightAddress.GetAddress() >= physAddr &&
		viewState.HighlightAddress.GetAddress() < physAddr + item.Item->ByteSize;

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, bHighlight ? Colours::highlight : Colours::address);
	ImGui::Text("%s", NumStr(physAddr));
	ImGui::PopStyleColor();

	const ENumberDisplayMode savedDisplayMode = GetNumberDisplayMode();
	bool bShowItemLabel = false;
	bool bShowASCII = false;
	uint32_t valueColour = Colours::defaultValue;

	switch (pDataInfo->DisplayType)
	{
	case EDataItemDisplayType::Pointer:
	case EDataItemDisplayType::JumpAddress:
		bShowItemLabel = true;
		break;
	case EDataItemDisplayType::Decimal:
		SetNumberDisplayMode(ENumberDisplayMode::Decimal);
		break;
	case EDataItemDisplayType::Binary:
		SetNumberDisplayMode(ENumberDisplayMode::Binary);
		break;
	case EDataItemDisplayType::Hex:
		SetNumberDisplayMode(GetHexNumberDisplayMode());
		break;
	case EDataItemDisplayType::Ascii:
		bShowASCII = true;
		break;
	case EDataItemDisplayType::Unknown:
		if (!pDataInfo->Writes.IsEmpty())
			valueColour = Colours::unknownDataWrite;
		else if (!pDataInfo->Reads.IsEmpty())
			valueColour = Colours::unknownDataRead;
		else
			valueColour = Colours::unknownValue;
		bShowASCII = true;
		break;
	default:
		break;
	}

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, valueColour);

	switch (pDataInfo->DataType)
	{
	case EDataType::Byte:
	case EDataType::InstructionOperand:
	{
		const uint8_t val = state.ReadByte(item.AddressRef);
		ImGui::Text("db");
		ImGui::SameLine();
		ImGui::Text("%s", NumStr(val));
		if (bShowASCII)
		{
			ImGui::SameLine();
			if (val == '\n')
				ImGui::Text("'<cr>'");
			else
				ImGui::Text("'%c'", val);
		}
		break;
	}
	case EDataType::Word:
	{
		const uint16_t val = state.ReadWord(physAddr);
		ImGui::Text("dw");
		ImGui::SameLine();
		ImGui::Text("%s", NumStr(val));
		if (bShowItemLabel)
		{
			ImGui::SameLine();
			DrawAddressLabel(state, viewState, pDataInfo->PointerAddress);
		}
		break;
	}
	case EDataType::Text:
	{
		const std::string textString = GetItemText(state, item.AddressRef);
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::defaultValue);
		ImGui::Text("ascii");
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::text);
		ImGui::Text("'%s'", textString.c_str());
		ImGui::PopStyleColor();
		break;
	}
	default:
		ImGui::Text("%d Bytes", pDataInfo->ByteSize);
		break;
	}

	ImGui::PopStyleColor();
	SetNumberDisplayMode(savedDisplayMode);
}

static void DrawZeroPageDataItem(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FCodeAnalysisItem& item)
{
	viewState.pLabelScope = state.GetScopeForAddress(item.AddressRef);
	ImGui::PushID(item.Item);
	ImGui::Selectable("##zpitem", false);
	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		viewState.GoToAddress(item.AddressRef);
	ImGui::SetItemAllowOverlap();
	ImGui::SameLine();
	DrawZeroPageDataInfo(state, viewState, item);
	ImGui::PopID();
}

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

	// One row per data item, with the preceding label (if any) merged in so the row count is known up front for the clipper.
	struct FZeroPageRow
	{
		const FCodeAnalysisItem* pDataItem = nullptr;
		const FCodeAnalysisItem* pLabelItem = nullptr;
	};

	std::vector<FZeroPageRow> zeroPageRows;
	const FCodeAnalysisItem* pPendingLabel = nullptr;
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

		if (item.Item->Type == EItemType::Label)
		{
			pPendingLabel = &item;
			continue;
		}

		if (item.Item->Type == EItemType::Data) // will zero page ever contain code?
			numLocationsDisplayed++;

		zeroPageRows.push_back({ &item, pPendingLabel });
		pPendingLabel = nullptr;
	}

	ImGui::Text("Showing: %d locations", numLocationsDisplayed);
	ImGui::Separator();

	if (ImGui::BeginTable("##zptable", 3, ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("##label",   ImGuiTableColumnFlags_WidthFixed,   0.0f);
		ImGui::TableSetupColumn("##data",    ImGuiTableColumnFlags_WidthFixed,   0.0f);
		ImGui::TableSetupColumn("##comment", ImGuiTableColumnFlags_WidthStretch);

		ImGuiListClipper clipper;
		clipper.Begin((int)zeroPageRows.size());
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FZeroPageRow& row = zeroPageRows[i];

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				const float cursorX = ImGui::GetCursorPosX();
				ShowDataItemActivity(state, row.pDataItem->AddressRef);

				if (row.pLabelItem != nullptr)
				{
					ImGui::SameLine(cursorX + ImGui::GetTextLineHeight() * 1.5f);
					DrawZeroPageLabel(state, viewState, *row.pLabelItem);
				}
				ImGui::TableSetColumnIndex(1);
				DrawZeroPageDataItem(state, viewState, *row.pDataItem);
				ImGui::TableSetColumnIndex(2);
				ImGui::PushID(row.pDataItem->Item);
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputText("##comment", &row.pDataItem->Item->Comment);
				ImGui::PopID();
			}
		}

		ImGui::EndTable();
	}

}
