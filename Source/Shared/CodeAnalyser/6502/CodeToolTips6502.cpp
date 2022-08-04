#include "CodeToolTips6502.h"
#include "../CodeAnalyser.h"
#include "../CodeToolTips.h"

#include <imgui.h>

static InstructionInfoMap g_InstructionInfo =
{
};

void ShowCodeToolTip6502(FCodeAnalysisState& state, const FCodeInfo* pCodeInfo)
{
	const uint8_t instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address);
	InstructionInfoMap::const_iterator it = g_InstructionInfo.find(instrByte);
	if (it == g_InstructionInfo.end())
		return;

	ImGui::BeginTooltip();
	ImGui::Text(it->second);
	ImGui::EndTooltip();
}