#include "DataItemZ80.h"

#include "../../CodeAnalyser.h"
#include <chips/z80.h>
#include <imgui.h>
#include "../UIColours.h"

bool DrawDataItemRegisterPtrsZ80(FCodeAnalysisState& state, uint16_t physAddr, uint16_t itemSize)
{
	std::string str;
	const uint16_t addrMax = physAddr + itemSize;
	if (state.CPUInterface->GetSP() >= physAddr && state.CPUInterface->GetSP() < addrMax)
	{
		str += "SP";
	}

	const z80_t* pCPU = (z80_t*)state.GetCPUInterface()->GetCPUEmulator()->GetImpl();

	if (pCPU->bc >= physAddr && pCPU->bc < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "BC";
	}

	if (pCPU->de >= physAddr && pCPU->de < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "DE";
	}

	if (pCPU->hl >= physAddr && pCPU->hl < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "HL";
	}

	if (pCPU->ix >= physAddr && pCPU->ix < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "IX";
	}

	if (pCPU->iy >= physAddr && pCPU->iy < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "IY";
	}

	if (pCPU->bc2 >= physAddr && pCPU->bc2 < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "BC'";
	}

	if (pCPU->de2 >= physAddr && pCPU->de2 < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "DE'";
	}

	if (pCPU->hl2 >= physAddr && pCPU->hl2 < addrMax)
	{
		str += str.empty() ? "" : ", ";
		str += "HL'";
	}

	if (!str.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, Colours::defaultValue);

		ImGui::SameLine();
		ImGui::Text("<- %s", str.c_str());
		ImGui::PopStyleColor();

		return true;
	}
	return false;
}