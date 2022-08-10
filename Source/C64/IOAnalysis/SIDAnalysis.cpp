#include "SIDAnalysis.h"

void	FSIDAnalysis::Init(FCodeAnalysisState* pAnalysis)
{

}

void	FSIDAnalysis::OnRegisterRead(uint8_t reg, uint16_t pc)
{

}
void	FSIDAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc)
{
	FC64IORegisterInfo& sidRegister = SIDRegisters[reg];
	const uint8_t regChange = sidRegister.LastVal ^ val;	// which bits have changed

	sidRegister.Accesses[pc].WriteVals.insert(val);

	sidRegister.LastVal = val;
}

void	FSIDAnalysis::DrawUI(void)
{

}