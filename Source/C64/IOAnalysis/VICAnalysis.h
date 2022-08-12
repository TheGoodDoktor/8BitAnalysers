#pragma once

#include "IORegisterAnalysis.h"

struct FCodeAnalysisState;

class FVICAnalysis
{
public:
	void	Init(FCodeAnalysisState* pAnalysis);
	void	OnRegisterRead(uint8_t reg, uint16_t pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc);

	void	DrawUI(void);

private:
	FC64IORegisterInfo	VICRegisters[64];

	int		SelectedRegister = -1;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
};