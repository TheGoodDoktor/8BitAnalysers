#include "JoypadViewer.h"

#include <imgui.h>

#include "../PCEEmu.h"
#include <geargrafx_core.h>


FJoypadViewer::FJoypadViewer(FEmuBase* pEmu)
: FViewerBase(pEmu) 
{ 
	Name = "Joypad";
	pPCEEmu = static_cast<FPCEEmu*>(pEmu);
}

bool FJoypadViewer::Init()
{
	return true;
}

void FJoypadViewer::DrawUI()
{
	/*
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	
	ImGui::Button(" U ");
	ImGui::SameLine();
	ImGui::Button(" D ");
	ImGui::SameLine();
	ImGui::Button(" L ");
	ImGui::SameLine();
	ImGui::Button(" R ");

	ImGui::Button("SEL");
	ImGui::SameLine();
	if (ImGui::Selectable("RUN", &bRunPressed))
	{
		if (bRunPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_RUN);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_RUN);
	}
	ImGui::SameLine();
	ImGui::Button("I  ");
	ImGui::SameLine();
	ImGui::Button("II ");
	*/
}
