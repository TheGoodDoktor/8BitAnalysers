#include "IORegisterAnalysis.h"

#include <imgui.h>
#include <vector>
#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>

void DrawRegValueHex(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("$%X", val);
}

void DrawRegValueDecimal(FC64IODevice* pDevice, uint8_t val)
{
	ImGui::Text("%d", val);
}

int DrawRegSelectList(std::vector<FRegDisplayConfig>& regList, int selection)
{
	for (int i = 0; i < (int)regList.size(); i++)
	{
		char selectableTXT[32];
		snprintf(selectableTXT, sizeof(selectableTXT), "$%X %s", i, regList[i].Name);
		if (ImGui::Selectable(selectableTXT, selection == i))
		{
			selection = i;
		}
	}

	return selection;
}

void DrawRegDetails(FC64IODevice* pDevice, FC64IORegisterInfo& reg, const FRegDisplayConfig& regConfig, FCodeAnalysisState* pCodeAnalysis)
{
	if (ImGui::Button("Clear"))
	{
		reg.LastVal = 0;
		reg.Accesses.clear();
	}
	// move out into function?
	ImGui::Text("Last Val:");
	regConfig.UIDrawFunction(pDevice, reg.LastVal);
	ImGui::Text("Accesses:");
	for (auto& access : reg.Accesses)
	{
		ImGui::Separator();
		ShowCodeAccessorActivity(*pCodeAnalysis, access.first);

		ImGui::Text("   ");
		ImGui::SameLine();
		DrawCodeAddress(*pCodeAnalysis, pCodeAnalysis->GetFocussedViewState(), access.first);

		if(ImGui::CollapsingHeader("Values"))
		{ 
			for (auto& val : access.second.WriteVals)
				regConfig.UIDrawFunction(pDevice, val);
		}
	}
}
