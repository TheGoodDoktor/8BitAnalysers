#include "DataItemZ80.h"

#include "../../CodeAnalyser.h"
#include <chips/z80.h>
#include <imgui.h>

void DrawDataItemRegisterPtrsZ80(FCodeAnalysisState& state, uint16_t physAddr)
{
	std::string str;

	if (state.CPUInterface->GetSP() == physAddr)
	{
		str += "SP";
	}

	z80_t* pCPU = (z80_t*)state.GetCPUInterface()->GetCPUEmulator();

	if (pCPU->bc == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "BC";
	}

	if (pCPU->de == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "DE";
	}

	if (pCPU->hl == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "HL";
	}

	if (pCPU->ix == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "IX";
	}

	if (pCPU->iy == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "IY";
	}

	if (pCPU->bc2 == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "BC'";
	}

	if (pCPU->de2 == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "DE'";
	}

	if (pCPU->hl2 == physAddr)
	{
		str += str.empty() ? "" : ", ";
		str += "HL'";
	}

	if (!str.empty())
	{
		ImGui::SameLine();
		ImGui::Text("<- %s", str.c_str());
	}
}