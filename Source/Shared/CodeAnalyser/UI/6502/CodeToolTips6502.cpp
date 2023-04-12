#include "CodeToolTips6502.h"
#include "../../CodeAnalyser.h"
#include "../CodeToolTips.h"

#include <imgui.h>

static InstructionInfoMap g_InstructionInfo =
{
};

void ShowCodeToolTip6502(FCodeAnalysisState& state, uint16_t addr)
{
	const uint8_t instrByte = state.ReadByte(addr);
	InstructionInfoMap::const_iterator it = g_InstructionInfo.find(instrByte);
	if (it == g_InstructionInfo.end())
		return;

	const FInstructionInfo& info = it->second;

	ImGui::BeginTooltip();
	ImGui::Text(info.Description);
	ImGui::EndTooltip();
}