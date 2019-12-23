#include "FunctionHandlers.h"
#include "SpeccyUI.h"
#include <algorithm>
#include "misc/cpp/imgui_stdlib.h"



/*bool GetROMLabelName(uint16_t callAddr, std::string &labelName)
{
	// Try to find a ROM function from the list
	const auto &romFunctionIt = g_RomLabels.find(callAddr);
	if (romFunctionIt != g_RomLabels.end())
	{
		labelName = romFunctionIt->second;
		return true;
	}

	return false;
}*/

int FunctionTrapFunction(uint16_t pc, uint16_t nextpc, int ticks, uint64_t pins, FSpeccyUI *pUI)
{
	// TODO: find functions
	uint8_t instrByte = ReadSpeccyByte(pUI->pSpeccy, pc);
	bool bCall = false;
	bool bRet = false;
	uint16_t callAddr = 0;

	switch (instrByte)
	{
		// RST Functions?
	case 0xc7:	// RST 0x00
		callAddr = 0x00;
		bCall = true;
		break;
	case 0xcf:	// RST 0x08
		callAddr = 0x08;
		bCall = true;
		break;
	case 0xd7:	// RST 0x10
		callAddr = 0x10;
		bCall = true;
		break;
	case 0xdf:	// RST 0x18
		callAddr = 0x18;
		bCall = true;
		break;
	case 0xe7:	// RST 0x20
		callAddr = 0x20;
		bCall = true;
		break;
	case 0xef:	// RST 0x28
		callAddr = 0x28;
		bCall = true;
		break;
	case 0xf7:	// RST 0x30
		callAddr = 0x30;
		bCall = true;
		break;
	case 0xff:	// RST 0x38
		callAddr = 0x38;
		bCall = true;
		break;

		// Call Functions
	case 0xcc:	// call z
	case 0xcd:	// call z
	case 0xdc:	// call c
	case 0xec:	// call pe
	case 0xfc:	// call m
	{
		callAddr = ReadSpeccyByte(pUI->pSpeccy, pc + 1);
		callAddr |= ReadSpeccyByte(pUI->pSpeccy, pc + 2) << 8;
		bCall = true;
	}
	break;

	// Return functions
	case 0xc0:	// ret nz
	case 0xd0:	// ret nc
	case 0xe0:	// ret po
	case 0xf0:	// ret p
	case 0xc8:	// ret z
	case 0xd8:	// ret c
	case 0xe8:	// ret pe
	case 0xf8:	// ret m
	case 0xc9:	// ret
		if (nextpc != pc + 1)	// have we actually returned?
			bRet = true;
		break;

	}

	// are we calling a function?
	if (bCall)
	{
		if (nextpc == callAddr)	// did call succeed?
		{
			FFunctionInfo &functionInfo = pUI->Functions[callAddr];
			if (functionInfo.NoCalls == 0)	// newly created
			{
				FLabelInfo *pLabelInfo = pUI->CodeAnalysis.Labels[callAddr];
				if (pLabelInfo!=nullptr)
					functionInfo.FunctionName = pLabelInfo->Name;
				
				functionInfo.StartAddress = callAddr;
				functionInfo.EndAddress = callAddr;
			}

			functionInfo.Callers[pc]++;
			functionInfo.NoCalls++;
			pUI->FunctionStack.push_back(callAddr);
			//return UI_DBG_STEP_TRAPID;
		}
	}

	if(bRet)
	{
		if (pUI->FunctionStack.size() > 0)
		{
			const uint16_t callAddr = pUI->FunctionStack.back();
			FFunctionInfo &functionInfo = pUI->Functions[callAddr];
			functionInfo.ExitPoints[pc]++;
			functionInfo.EndAddress = std::max(functionInfo.EndAddress, pc);
			pUI->FunctionStack.pop_back();
		}
		//return UI_DBG_STEP_TRAPID;
	}

	return 0;
}

void DrawFunctionInfo(FSpeccyUI* pUI)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("DrawFunctionInfoGUIChild1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), false, window_flags);
	FFunctionInfo *pSelectedFunction = nullptr;

	for (auto &functionIt : pUI->Functions)
	{
		FFunctionInfo&function = functionIt.second;
		const bool bSelected = pUI->SelectedFunction == functionIt.first;
		if (bSelected)
		{
			pSelectedFunction = &function;
		}

		if (ImGui::Selectable(function.FunctionName.c_str(), bSelected))
		{
			pUI->SelectedFunction = functionIt.first;
			pUI->FunctionDasm.StartAddress = function.StartAddress;
		}

	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Handler details
	ImGui::BeginChild("DrawFunctionInfoGUIChild2", ImVec2(0, 0), false, window_flags);
	if (pSelectedFunction != nullptr)
	{
		//ImGui::Checkbox("Enabled", &pSelectedHandler->bEnabled);
		//ImGui::Checkbox("Break", &pSelectedHandler->bBreak);
		//ImGui::Text(pSelectedFunction->FunctionName.c_str());

		if (ImGui::InputText("Name:", &pSelectedFunction->FunctionName))
		{
		}
		ImGui::Text("0x%x - 0x%x", pSelectedFunction->StartAddress, pSelectedFunction->EndAddress);
		ImGui::Text("Total Accesses %d", pSelectedFunction->NoCalls);

		ImGui::Text("Callers");
		for (const auto &accessPC : pSelectedFunction->Callers)
		{
			ImGui::PushID(accessPC.first);
			ImGui::Text("0x%x - %d accesses", accessPC.first, accessPC.second);
			ImGui::SameLine();
			if (ImGui::ArrowButton("GotoAddr", ImGuiDir_Right))
			{
				pUI->FunctionDasm.StartAddress = accessPC.first;
			}
			ImGui::PopID();
		}

		ImGui::Text("Exit Points");
		for (const auto &accessPC : pSelectedFunction->ExitPoints)
		{
			ImGui::PushID(accessPC.first);
			ImGui::Text("0x%x - %d accesses", accessPC.first, accessPC.second);
			ImGui::SameLine();
			if (ImGui::ArrowButton("GotoAddr", ImGuiDir_Right))
			{
				pUI->FunctionDasm.StartAddress = accessPC.first;
			}
			ImGui::PopID();
		}

		pUI->FunctionDasm.open = true;
	}
	ImGui::EndChild();
}