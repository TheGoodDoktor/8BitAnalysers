#pragma once

#include "CodeAnalyser/IOAnalyser.h"
#include "IORegisterAnalysis.h"
#include <vector>

class FCodeAnalysisState;
struct FCodeAnalysisPage;

class FCIAAnalysis : public FIODevice
{
public:
	void	Init(FCodeAnalysisState* pAnalysis);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);

	void	DrawDetailsUI(void);

private:
	static const int kNoRegisters = 16;
	FC64IORegisterInfo	CIARegisters[kNoRegisters];

	int		SelectedRegister = -1;
protected:
	std::vector<FRegDisplayConfig>* RegConfig = nullptr;
};

class FCIA1Analysis : public FCIAAnalysis
{
public:
	FCIA1Analysis();
};

class FCIA2Analysis : public FCIAAnalysis
{
public:
	FCIA2Analysis();
};

void AddCIARegisterLabels(FCodeAnalysisPage& IOPage);