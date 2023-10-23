#pragma once

#include "CodeAnalyser/IOAnalyser.h"

#include "IORegisterAnalysis.h"

class FCodeAnalysisState;
struct FCodeAnalysisPage;

class FVICAnalysis : public FIODevice
{
public:
	void	Init(FCodeAnalysisState* pAnalysis);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);

	void	DrawDetailsUI(void);

private:
	static const int kNoRegisters = 64;
	FC64IORegisterInfo	VICRegisters[kNoRegisters];

	int		SelectedRegister = -1;
};

void AddVICRegisterLabels(FCodeAnalysisPage& IOPage);