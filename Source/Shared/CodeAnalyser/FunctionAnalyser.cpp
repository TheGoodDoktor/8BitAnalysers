#include "FunctionAnalyser.h"

#include "CodeAnalyser.h"
#include "UI/DisplayTypes.h"

FFunctionParam::FFunctionParam()
{
	Name = "Param";
	pDisplayType = GetDisplayType("Unknown");
	for(int i=0;i<kMaxHistory;i++)
		History[i] = 0;
}

std::map< EFunctionParamSourceZ80, const char*> g_Z80RegNames =
{
	{ EFunctionParamSourceZ80::RegA, "A" },
	{ EFunctionParamSourceZ80::RegB, "B" },
	{ EFunctionParamSourceZ80::RegC, "C" },
	{ EFunctionParamSourceZ80::RegD, "D" },
	{ EFunctionParamSourceZ80::RegE, "E" },
	{ EFunctionParamSourceZ80::RegH, "H" },
	{ EFunctionParamSourceZ80::RegL, "L" },
	{ EFunctionParamSourceZ80::RegBC, "BC" },
	{ EFunctionParamSourceZ80::RegDE, "DE" },
	{ EFunctionParamSourceZ80::RegHL, "HL" },
	{ EFunctionParamSourceZ80::RegIX, "IX" },
	{ EFunctionParamSourceZ80::RegIY, "IY" },
};

std::map< EFuctionParamSourceM6502, const char*> g_M6502RegNames =
{
	{ EFuctionParamSourceM6502::RegA, "A" },
	{ EFuctionParamSourceM6502::RegX, "X" },
	{ EFuctionParamSourceM6502::RegY, "Y" },
};

std::string FFunctionParam::GenerateDescription(FCodeAnalysisState& state) const
{
	std::string regName;
	if (state.GetCPUInterface()->CPUType == ECPUType::Z80)
	{
		auto findIt = g_Z80RegNames.find(Z80Source);
		if(findIt != g_Z80RegNames.end())
			regName = findIt->second;
	}
	else if (state.GetCPUInterface()->CPUType == ECPUType::M6502)
	{
		auto findIt = g_M6502RegNames.find(M6502Source);
		if (findIt != g_M6502RegNames.end())
			regName = findIt->second;
	}

	return regName + " : " + Name;
}


void CaptureFunctionParam(FCodeAnalysisState& state, FFunctionParam& param)
{
	uint16_t value = 0;

	if (state.GetCPUInterface()->CPUType == ECPUType::Z80)
	{
		const z80_t* pZ80 = (const z80_t*)state.GetCPUInterface()->GetCPUEmulator();

		switch (param.Z80Source)
		{
		case EFunctionParamSourceZ80::RegA:
			value = pZ80->a;
			break;
		case EFunctionParamSourceZ80::RegB:
			value = pZ80->b;
			break;
		case EFunctionParamSourceZ80::RegC:
			value = pZ80->c;
			break;
		case EFunctionParamSourceZ80::RegD:
			value = pZ80->d;
			break;
		case EFunctionParamSourceZ80::RegE:
			value = pZ80->e;
			break;
		case EFunctionParamSourceZ80::RegH:
			value = pZ80->h;
			break;
		case EFunctionParamSourceZ80::RegL:
			value = pZ80->l;
			break;
		case EFunctionParamSourceZ80::RegBC:
			value = pZ80->bc;
			break;
		case EFunctionParamSourceZ80::RegDE:
			value = pZ80->de;
			break;
		case EFunctionParamSourceZ80::RegHL:
			value = pZ80->hl;
			break;
		case EFunctionParamSourceZ80::RegIX:
			value = pZ80->ix;
			break;
		case EFunctionParamSourceZ80::RegIY:
			value = pZ80->iy;
			break;
		}
	}
	else if (state.GetCPUInterface()->CPUType == ECPUType::M6502)
	{
		const m6502_t* pM6502 = (const m6502_t*)state.GetCPUInterface()->GetCPUEmulator();
		switch (param.M6502Source)
		{
		case EFuctionParamSourceM6502::RegA:
			value = pM6502->A;
			break;
		case EFuctionParamSourceM6502::RegX:
			value = pM6502->X;
			break;
		case EFuctionParamSourceM6502::RegY:
			value = pM6502->Y;
			break;
		}
	}

	param.StoreValue(value);
}

void FFunctionInfo::OnCalled(FCodeAnalysisState& state)
{
	// Capture parameters
	for (auto& param : Params)
	{
		CaptureFunctionParam(state, param);
	}
	
}

void FFunctionInfo::OnReturned(FCodeAnalysisState& state)
{
	// capture return values
	for (auto& returnValue : ReturnValues)
	{
		CaptureFunctionParam(state, returnValue);
	}
}

FFunctionInfo* FFunctionInfoCollection::FindFunctionByName(const char* name)
{
	for (auto& funcPair : Functions)
	{
		FLabelInfo* pLabel = pAnalyser->GetLabelForAddress(funcPair.second.StartAddress);
		if (pLabel != nullptr && strcmp(pLabel->GetName(), name) == 0)
			return &funcPair.second;
	}
	return nullptr;
}