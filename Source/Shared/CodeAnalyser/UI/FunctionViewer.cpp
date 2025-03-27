#include "FunctionViewer.h"

#include <imgui.h>
#include "CodeAnalyserUI.h"
#include "ImGuiSupport/ImGuiScaling.h"

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
		for (const auto& functionIt : state.Functions.GetFunctions())
		{
			const FFunctionInfo& function = functionIt.second;

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
		auto pFunctionInfo = state.Functions.GetFunctionAtAddress(SelectedFunction);
		if (pFunctionInfo)
		{
			DrawFunctionDetails(pFunctionInfo);
		}
		else
		{
			ImGui::Text("No function selected");
		}
	}
	ImGui::EndChild();
}

void FFunctionViewer::DrawFunctionDetails(FFunctionInfo* pFunctionInfo)
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	FLabelInfo* pLabelInfo = state.GetLabelForAddress(pFunctionInfo->StartAddress);
	ImGui::Text("%s", pFunctionInfo->Name.c_str());
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
		const float glyphWidth = ImGui_GetFontCharWidth();
		ImGui::SetNextItemWidth(glyphWidth * 10.0f);
		ImGui::InputScalar("End Address", ImGuiDataType_U16, &pFunctionInfo->EndAddress.Address, nullptr, nullptr, format, inputFlags);
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
			DrawCodeAddress(state, viewState, ref);
			
			ImGui::PopID();
		}
	}

	// Draw list of callers
	if(pFunctionInfo->CallPoints.size() != 0)
	{
		ImGui::Text("Called Functions:");
		for (const auto& calledFunction : pFunctionInfo->CallPoints)
		{
			ImGui::Text("\t");
			DrawAddressLabel(state, viewState, calledFunction.FunctionAddr);
			ImGui::SameLine();
			ImGui::Text(" at ");
			DrawAddressLabel(state, viewState, calledFunction.CallAddr);
		}
	}

	// Draw list of called functions
	if(pFunctionInfo->ExitPoints.size() != 0)
	{
		ImGui::Text("Exit points:");
		// Draw list of exit points
		for (auto& exitPoint : pFunctionInfo->ExitPoints)
		{
			ImGui::Text("\t");
			DrawAddressLabel(state, viewState, exitPoint);
		}
	}
}


void FFunctionViewer::DrawUI()
{
	FCodeAnalysisState& state = pEmulator->GetCodeAnalysis();
	ImGui::Text("Functions");
	ImGui::SameLine();
	ImGui::Checkbox("Trace Execution", &state.bTraceFunctionExecution);
	DrawFunctionList();
}

