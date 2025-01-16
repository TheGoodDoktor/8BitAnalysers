#include "IORegisterAnalysis.h"

#include <imgui.h>
#include <vector>
#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/UI/CodeAnalyserUI.h>
#include "../BBCChipsImpl.h"


void	FBBCIODevice::OnRegisterRead(uint8_t reg, FAddressRef pc)
{
	//pCodeAnalyser->Debugger.RegisterEvent(ReadEventType, pc, reg, RegisterInfo[reg].LastVal, 0);

}

void	FBBCIODevice::OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc)
{
	RegisterInfo[reg].LastVal = val;
	RegisterInfo[reg].Accesses[pc].WriteVals.insert(val);

	//pCodeAnalyser->Debugger.RegisterEvent(WriteEventType, pc, reg, val, 0);

}

void FBBCIODevice::DrawDetailsUI(void)
{
	if(pRegConfig == nullptr)
		return;

	if (ImGui::BeginChild("Reg Select", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true))
	{
		SelectedRegister = DrawRegSelectList(*pRegConfig, SelectedRegister);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("Reg Details"))
	{
		if (SelectedRegister != -1)
		{
			ImGui::PushID(SelectedRegister);
			FRegDisplayConfig& regConfig = pRegConfig->at(SelectedRegister);
			DrawRegDetails(this, RegisterInfo[regConfig.Address & 0xff], regConfig, pCodeAnalyser);
			ImGui::PopID();  
		}
	}
	ImGui::EndChild();
}

void DrawRegValueHex(FBBCIODevice* pDevice, uint8_t val)
{
	ImGui::Text("$%X", val);
}

void DrawRegValueDecimal(FBBCIODevice* pDevice, uint8_t val)
{
	ImGui::Text("%d", val);
}

int DrawRegSelectList(std::vector<FRegDisplayConfig>& regList, int selection)
{
	for (int i = 0; i < (int)regList.size(); i++)
	{
		char selectableTXT[32];
		snprintf(selectableTXT, sizeof(selectableTXT), "&%X : %s", regList[i].Address, regList[i].Name);
		if (ImGui::Selectable(selectableTXT, selection == i))
		{
			selection = i;
		}
	}

	return selection;
}

void DrawRegDetails(FBBCIODevice* pDevice, FBBCIORegisterInfo& reg, const FRegDisplayConfig& regConfig, FCodeAnalysisState* pCodeAnalysis)
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
	int imguiId = 0;
	for (auto& access : reg.Accesses)
	{
		ImGui::PushID(imguiId++);
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
		ImGui::PopID();
	}
}
