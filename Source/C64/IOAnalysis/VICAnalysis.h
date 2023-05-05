#pragma once

#include "IORegisterAnalysis.h"

class FCodeAnalysisState;
struct FCodeAnalysisPage;

class FVICAnalysis
{
public:
	void	Init(FCodeAnalysisState* pAnalysis);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, uint16_t pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc);

	void	DrawUI(void);

private:
	static const int kNoRegisters = 64;
	FC64IORegisterInfo	VICRegisters[kNoRegisters];

	int		SelectedRegister = -1;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
};

void AddVICRegisterLabels(FCodeAnalysisPage& IOPage);