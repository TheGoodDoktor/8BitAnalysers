#pragma once

#include "IORegisterAnalysis.h"
#include <vector>

class FCodeAnalysisState;
struct FCodeAnalysisPage;

class FCIAAnalysis
{
public:
	void	Init(FCodeAnalysisState* pAnalysis);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, uint16_t pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, uint16_t pc);

	void	DrawUI(void);

private:
	static const int kNoRegisters = 16;
	FC64IORegisterInfo	CIARegisters[kNoRegisters];

	int		SelectedRegister = -1;

	FCodeAnalysisState* pCodeAnalysis = nullptr;
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