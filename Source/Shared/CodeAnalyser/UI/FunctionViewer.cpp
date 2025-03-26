#include "FunctionViewer.h"

#include <imgui.h>
#include "CodeAnalyserUI.h"

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
	ImGui::Text("Start Address");
	DrawAddressLabel(state,viewState,pFunctionInfo->StartAddress);
	ImGui::Text("End Address");
	DrawAddressLabel(state, viewState, pFunctionInfo->EndAddress);

	if(pLabelInfo != nullptr)
	{
		ImGui::Text("Callers:");
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

	// TODO: draw list of callers
	ImGui::Text("Called Functions:");
	for (const auto& calledFunction : pFunctionInfo->CallPoints)
	{
		ImGui::Text("Called Function:");
		DrawAddressLabel(state, viewState, calledFunction.FunctionAddr);
	}
	// TODO: draw list of called functions
	ImGui::Text("Exit points:");
	// Draw list of exit points
	for (auto& exitPoint : pFunctionInfo->ExitPoints)
	{
		ImGui::Text("Exit Point:");
		DrawAddressLabel(state, viewState, exitPoint);
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

