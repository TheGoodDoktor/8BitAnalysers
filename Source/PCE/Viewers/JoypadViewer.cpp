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
	FCodeAnalysisState& state = pPCEEmu->GetCodeAnalysis();
	
	if (ImGui::Selectable("Up", &bUpPressed))
	{
		if (bUpPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_UP);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_UP);
	}
	if (ImGui::Selectable("Down", &bDownPressed))
	{
		if (bDownPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_DOWN);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_DOWN);
	}
	if (ImGui::Selectable("Left", &bLeftPressed))
	{
		if (bLeftPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_LEFT);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_LEFT);
	}
	if (ImGui::Selectable("Right", &bRightPressed))
	{
		if (bRightPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_RIGHT);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_RIGHT);
	}
		if (ImGui::Selectable("Select", &bSelectPressed))
	{
		if (bSelectPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_SELECT);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_SELECT);
	}
	if (ImGui::Selectable("Run", &bRunPressed))
	{
		if (bRunPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_RUN);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_RUN);
	}
	if (ImGui::Selectable("I", &bIPressed))
	{
		if (bIPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_I);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_I);
	}
	if (ImGui::Selectable("II", &bIIPressed))
	{
		if (bIIPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, GG_KEY_II);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, GG_KEY_II);
	}
}
