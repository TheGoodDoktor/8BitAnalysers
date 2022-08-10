#pragma once

#include "IORegisterAnalysis.h"

struct FCodeAnalysisState;

class FCIAAnalysis
{
public:
	void	Init(FCodeAnalysisState* pAnalysis);
	void	OnRegisterRead(uint8_t reg, uint16_t pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc);

	void	DrawUI(void);

private:
	FC64IORegisterInfo	CIARegisters[16];

	int		UICIARegister = -1;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
};

class FCIA1Analysis : public FCIAAnalysis
{

};

class FCIA2Analysis : public FCIAAnalysis
{

};