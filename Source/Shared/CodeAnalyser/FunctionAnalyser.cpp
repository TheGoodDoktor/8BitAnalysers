#include "FunctionAnalyser.h"

#include "CodeAnalyser.h"

void FFunctionInfo::OnCalled(FCodeAnalysisState& state)
{
	// Capture parameters
	for (auto& param : Params)
	{
		if (state.GetCPUInterface()->CPUType == ECPUType::Z80)
		{
			const z80_t* pZ80 = (const z80_t*)state.GetCPUInterface()->GetCPUEmulator();

			switch (param.Z80Source)
			{
			case EFunctionParamSourceZ80::RegA:
				param.LastValue = pZ80->a;
				break;
			case EFunctionParamSourceZ80::RegB:
				param.LastValue = pZ80->b;
				break;
			case EFunctionParamSourceZ80::RegC:
				param.LastValue = pZ80->c;
				break;
			case EFunctionParamSourceZ80::RegD:
				param.LastValue = pZ80->d;
				break;
			case EFunctionParamSourceZ80::RegE:
				param.LastValue = pZ80->e;
				break;
			case EFunctionParamSourceZ80::RegH:
				param.LastValue = pZ80->h;
				break;
			case EFunctionParamSourceZ80::RegL:
				param.LastValue = pZ80->l;
				break;
			case EFunctionParamSourceZ80::RegBC:
				param.LastValue = pZ80->bc;
				break;
			case EFunctionParamSourceZ80::RegDE:
				param.LastValue = pZ80->de;
				break;
			case EFunctionParamSourceZ80::RegHL:
				param.LastValue = pZ80->hl;
				break;
			case EFunctionParamSourceZ80::RegIX:
				param.LastValue = pZ80->ix;
				break;
			case EFunctionParamSourceZ80::RegIY:
				param.LastValue = pZ80->iy;
				break;
			}
		}
		else if (state.GetCPUInterface()->CPUType == ECPUType::M6502)
		{
			const m6502_t* pM6502 = (const m6502_t*)state.GetCPUInterface()->GetCPUEmulator();
			switch (param.M6502Source)
			{
			case EFuctionParamSourceM6502::RegA:
				param.LastValue = pM6502->A;
				break;
			case EFuctionParamSourceM6502::RegX:
				param.LastValue = pM6502->X;
				break;
			case EFuctionParamSourceM6502::RegY:
				param.LastValue = pM6502->Y;
				break;
			}
		}
	}
	
}