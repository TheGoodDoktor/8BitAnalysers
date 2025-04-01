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

			switch (param.Z80Type)
			{
			case EFunctionParamTypeZ80::RegA:
				param.LastValue = pZ80->a;
				break;
			case EFunctionParamTypeZ80::RegB:
				param.LastValue = pZ80->b;
				break;
			case EFunctionParamTypeZ80::RegC:
				param.LastValue = pZ80->c;
				break;
			case EFunctionParamTypeZ80::RegD:
				param.LastValue = pZ80->d;
				break;
			case EFunctionParamTypeZ80::RegE:
				param.LastValue = pZ80->e;
				break;
			case EFunctionParamTypeZ80::RegH:
				param.LastValue = pZ80->h;
				break;
			case EFunctionParamTypeZ80::RegL:
				param.LastValue = pZ80->l;
				break;
			case EFunctionParamTypeZ80::RegBC:
				param.LastValue = pZ80->bc;
				break;
			case EFunctionParamTypeZ80::RegDE:
				param.LastValue = pZ80->de;
				break;
			case EFunctionParamTypeZ80::RegHL:
				param.LastValue = pZ80->hl;
				break;
			case EFunctionParamTypeZ80::RegIX:
				param.LastValue = pZ80->ix;
				break;
			case EFunctionParamTypeZ80::RegIY:
				param.LastValue = pZ80->iy;
				break;
			}
		}
		else if (state.GetCPUInterface()->CPUType == ECPUType::M6502)
		{
			const m6502_t* pM6502 = (const m6502_t*)state.GetCPUInterface()->GetCPUEmulator();
			switch (param.M6502Type)
			{
			case EFuctionParamTypeM6502::RegA:
				param.LastValue = pM6502->A;
				break;
			case EFuctionParamTypeM6502::RegX:
				param.LastValue = pM6502->X;
				break;
			case EFuctionParamTypeM6502::RegY:
				param.LastValue = pM6502->Y;
				break;
			}
		}
	}
	
}