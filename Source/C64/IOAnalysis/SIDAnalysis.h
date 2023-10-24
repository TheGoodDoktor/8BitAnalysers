#pragma once

#include "CodeAnalyser/IOAnalyser.h"
#include "IORegisterAnalysis.h"

class FCodeAnalysisState;
struct FCodeAnalysisPage;

class FSIDAnalysis : public FC64IODevice
{
public:
	void	Init(FC64Emulator* pEmulator);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);

	void	DrawDetailsUI(void);

private:
	static const int kNoRegisters = 32;
	FC64IORegisterInfo	SIDRegisters[kNoRegisters];

	int		SelectedRegister = -1;
};

void AddSIDRegisterLabels(FCodeAnalysisPage& IOPage);