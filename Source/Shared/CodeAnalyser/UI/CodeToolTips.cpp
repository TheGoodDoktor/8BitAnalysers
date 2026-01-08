#include "CodeToolTips.h"
#include "../CodeAnalyser.h"
#include "Z80/CodeToolTipsZ80.h"
#include "6502/CodeToolTips6502.h"
#include "Debug/DebugLog.h"

void ShowCodeToolTip(FCodeAnalysisState& state, uint16_t addr)
{
	switch (state.CPUInterface->CPUType)
	{
	case ECPUType::Z80:
		ShowCodeToolTipZ80(state, addr);
		break;
	case ECPUType::M6502:
	case ECPUType::M65C02:
		ShowCodeToolTip6502(state, addr);
		break;
    default:
        LOGERROR("Unknow CPU");
        break;
	}
}
