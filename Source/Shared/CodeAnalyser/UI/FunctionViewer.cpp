#include "FunctionViewer.h"

#include <imgui.h>
#include "CodeAnalyserUI.h"
#include "ImGuiSupport/ImGuiScaling.h"
#include "../FunctionAnalyser.h"
#include "misc/cpp/imgui_stdlib.h"

bool FFunctionViewer::Init()
{
	return true;
}

void FFunctionViewer::Shutdown()
{
}

void FFunctionViewer::DrawFunctionList()
{ 
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::BeginChild("##itemselect", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), true))
	{
		// TODO: draw list of items as selectables
		for (const auto& functionIt : state.pFunctions->GetFunctions())
		{
			const FFunctionInfo& function = functionIt.second;

			if(bOnlyShowVisitedFunctions && function.IsVisited() == false)
				continue;

			ImGui::PushID(function.StartAddress.Address);
			if (ImGui::Selectable(function.Name.c_str(), SelectedFunction == function.StartAddress))
			{
				SelectedFunction = function.StartAddress;
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("##itemdetails", ImVec2(0, 0), true))
	{
		// Draw details of selected item
		auto pFunctionInfo = state.pFunctions->GetFunctionAtAddress(SelectedFunction);
		if (pFunctionInfo)
		{
			DrawFunctionDetails(pEmulator->GetCodeAnalysis(),pFunctionInfo);
		}
		else
		{
			ImGui::Text("No function selected");
		}
	}
	ImGui::EndChild();
}

void FFunctionViewer::DrawUI()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	ImGui::Text("Functions");
	ImGui::SameLine();
	ImGui::Checkbox("Trace Execution", &state.bTraceFunctionExecution);
	ImGui::Checkbox("Visited Functions Only", &bOnlyShowVisitedFunctions);
	DrawFunctionList();
}

// type string for EFunctionParamTypeZ80
static const char* g_ParamTypeZ80[] = 
{
	"RegA",
	"RegB",
	"RegC",
	"RegD",
	"RegE",
	"RegH",
	"RegL",
	"RegBC",
	"RegDE",
	"RegHL",
	"RegIX", 
	"RegIY",
};

// type string for EFuctionParamTypeM6502
static const char* g_ParamTypeM6502[] =
{
	"RegA",
	"RegX",
	"RegY",
};

void DrawParameterTypeComboBox_Z80(EFunctionParamTypeZ80& val)
{
	ImGui::Combo("##Type", (int*)&val, g_ParamTypeZ80, IM_ARRAYSIZE(g_ParamTypeZ80));
}

void DrawParameterTypeComboBox_M6502(EFuctionParamTypeM6502& val)
{
	ImGui::Combo("##Type", (int*)&val, g_ParamTypeM6502, IM_ARRAYSIZE(g_ParamTypeM6502));
}

void DrawFunctionDetails(FCodeAnalysisState& state, FFunctionInfo* pFunctionInfo)
{
	//FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	FLabelInfo* pLabelInfo = state.GetLabelForAddress(pFunctionInfo->StartAddress);
	const float glyphWidth = ImGui_GetFontCharWidth();
	//ImGui::Text("%s", pFunctionInfo->Name.c_str());
	ImGui::Text("Address range:");
	DrawAddressLabel(state,viewState,pFunctionInfo->StartAddress);
	ImGui::SameLine();
	ImGui::Text("-> ");
	DrawAddressLabel(state, viewState, pFunctionInfo->EndAddress);
	ImGui::Checkbox("Manual Edit", &pFunctionInfo->bManualEdit);
	if (pFunctionInfo->bManualEdit)
	{
		ImGui::SameLine();

		const ImGuiInputTextFlags inputFlags = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_CharsHexadecimal;
		const char* format = (GetNumberDisplayMode() == ENumberDisplayMode::Decimal) ? "%d" : "%04X";
		ImGui::SetNextItemWidth(glyphWidth * 10.0f);
		ImGui::InputScalar("End Address", ImGuiDataType_U16, &pFunctionInfo->EndAddress.Address, nullptr, nullptr, format, inputFlags);
	}

	// List Parameters - name, type, last value
	// maybe table would be better?
	ImGui::Text("Parameters:");
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable("Parameters", 4, flags))
	{
		const float charWidth = ImGui_GetFontCharWidth();
		ImGui::TableSetupColumn("No", ImGuiTableColumnFlags_WidthFixed, 4 * charWidth);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 20 * charWidth);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 10 * charWidth);
		ImGui::TableSetupColumn("Last Value", ImGuiTableColumnFlags_WidthFixed, 20 * charWidth);
		ImGui::TableHeadersRow();
		for (int paramNo = 0; paramNo < pFunctionInfo->Params.size(); paramNo++)
		{
			FFunctionParam& param = pFunctionInfo->Params[paramNo];
			ImGui::PushID(paramNo);
			ImGui::TableNextRow();

			// No
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d",paramNo);
			// Name
			ImGui::TableSetColumnIndex(1);
			//ImGui::SetNextItemWidth(glyphWidth * 20.0f);
			ImGui::InputText("##Name", &param.Name);

			// Type
			ImGui::TableSetColumnIndex(2);
			//ImGui::SetNextItemWidth(glyphWidth * 10.0f);
			if (state.CPUInterface->CPUType == ECPUType::Z80)
			{
				DrawParameterTypeComboBox_Z80(param.Z80Type);
			}
			else if (state.CPUInterface->CPUType == ECPUType::M6502)
			{
				DrawParameterTypeComboBox_M6502(param.M6502Type);
			}
			// Last Value
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%s", NumStr(param.LastValue));
			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	// Add Parameter
	if (ImGui::Button("Add Parameter"))
	{
		FFunctionParam newParam;
		newParam.Name = "NewParam";
		pFunctionInfo->Params.push_back(newParam);
	}

	if(pLabelInfo != nullptr)
	{
		ImGui::Text("Called By:");
		for (const auto& ref : pLabelInfo->References.GetReferences())
		{
			ImGui::PushID(ref.Val);
			ShowCodeAccessorActivity(state, ref);

			ImGui::Text("   ");
			ImGui::SameLine();
			DrawAddressLabel(state, viewState, ref);
			
			ImGui::PopID();
		}
	}

	// Draw list of callers
	if(pFunctionInfo->CallPoints.size() != 0)
	{
		ImGui::Text("Called Functions:");
		for (const auto& calledFunction : pFunctionInfo->CallPoints)
		{
			//ImGui::Text("\t");
			//ImGui::SameLine();
			ShowCodeAccessorActivity(state, calledFunction.CallAddr);
			ImGui::Text("   ");
			ImGui::SameLine();
			DrawAddressLabel(state, viewState, calledFunction.CallAddr);
			ImGui::SameLine();
			ImGui::Text(" calls ");
			DrawAddressLabel(state, viewState, calledFunction.FunctionAddr);
		}
	}

	// Draw list of called functions
	if(pFunctionInfo->ExitPoints.size() != 0)
	{
		ImGui::Text("Exit points:");
		// Draw list of exit points
		for (auto& exitPoint : pFunctionInfo->ExitPoints)
		{
			//ImGui::Text("\t");
			//ImGui::SameLine();
			ShowCodeAccessorActivity(state, exitPoint);
			ImGui::Text("   ");
			ImGui::SameLine();
			DrawAddressLabel(state, viewState, exitPoint);
		}
	}
}



