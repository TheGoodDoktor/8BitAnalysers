#include "CIAAnalysis.h"

void	FCIAAnalysis::Init(FCodeAnalysisState* pAnalysis)
{

}

void	FCIAAnalysis::OnRegisterRead(uint8_t reg, uint16_t pc)
{

}
void	FCIAAnalysis::OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc)
{
	FC64IORegisterInfo& sidRegister = CIARegisters[reg];
	const uint8_t regChange = sidRegister.LastVal ^ val;	// which bits have changed

	sidRegister.Accesses[pc].WriteVals.insert(val);

	sidRegister.LastVal = val;
}

void	FCIAAnalysis::DrawUI(void)
{

}