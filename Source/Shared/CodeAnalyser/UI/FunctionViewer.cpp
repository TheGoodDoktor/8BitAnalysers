#include "FunctionViewer.h"

#include <imgui.h>

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

}


void FFunctionViewer::DrawUI()
{
	ImGui::Text("Functions");
	DrawFunctionList();
}

