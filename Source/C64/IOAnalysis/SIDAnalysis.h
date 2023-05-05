#pragma once

#include "IORegisterAnalysis.h"

class FCodeAnalysisState;
struct FCodeAnalysisPage;

class FSIDAnalysis
{
public:
	void	Init(FCodeAnalysisState* pAnalysis);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, uint16_t pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc);

	void	DrawUI(void);

private:
	static const int kNoRegisters = 32;
	FC64IORegisterInfo	SIDRegisters[kNoRegisters];

	int		SelectedRegister = -1;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
};

void AddSIDRegisterLabels(FCodeAnalysisPage& IOPage);