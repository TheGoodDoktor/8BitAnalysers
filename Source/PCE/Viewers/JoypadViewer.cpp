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

static void DrawButton(const char* label, bool& bPressed, GG_Keys key, FPCEEmu* pPCEEmu)
{
	const bool bWasPressed = bPressed;
	if (bWasPressed)
	{
		const ImVec4 pressedColour(0.2f, 0.7f, 0.2f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Header, pressedColour);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, pressedColour);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, pressedColour);
	}

	const bool bChanged = ImGui::Selectable(label, &bPressed);

	if (bWasPressed)
		ImGui::PopStyleColor(3);

	if (bChanged)
	{
		if (bPressed)
			pPCEEmu->GetCore()->KeyPressed(GG_CONTROLLER_1, key);
		else
			pPCEEmu->GetCore()->KeyReleased(GG_CONTROLLER_1, key);
	}
}

void FJoypadViewer::DrawUI()
{
	DrawButton("Up", bUpPressed, GG_KEY_UP, pPCEEmu);
	DrawButton("Down", bDownPressed, GG_KEY_DOWN, pPCEEmu);
	DrawButton("Left", bLeftPressed, GG_KEY_LEFT, pPCEEmu);
	DrawButton("Right", bRightPressed, GG_KEY_RIGHT, pPCEEmu);
	DrawButton("Select", bSelectPressed, GG_KEY_SELECT, pPCEEmu);
	DrawButton("Run", bRunPressed, GG_KEY_RUN, pPCEEmu);
	DrawButton("I", bIPressed, GG_KEY_I, pPCEEmu);
	DrawButton("II", bIIPressed, GG_KEY_II, pPCEEmu);
}
