#include "CodeToolTips.h"
#include "../CodeAnalyser.h"
#include "Z80/CodeToolTipsZ80.h"
#include "6502/CodeToolTips6502.h"


void ShowCodeToolTip(FCodeAnalysisState& state, const FCodeInfo* pCodeInfo)
{
	if (state.CPUInterface->CPUType == ECPUType::Z80)
		ShowCodeToolTipZ80(state, pCodeInfo);
	else if (state.CPUInterface->CPUType == ECPUType::M6502)
		ShowCodeToolTip6502(state, pCodeInfo);
}