#pragma once

#include "IORegisterAnalysis.h"
#include <vector>

#include <chips/chips/m6526.h>

class FCodeAnalysisState;
struct FCodeAnalysisPage;
class FC64Emulator;

class FCIAAnalysis : public FC64IODevice
{
public:
	void	Init(FC64Emulator* pEmulator);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);

	void	DrawDetailsUI(void);

	virtual const m6526_t*	GetCIA() = 0;
private:
	static const int kNoRegisters = 16;
	FC64IORegisterInfo	CIARegisters[kNoRegisters];

	int		SelectedRegister = -1;
protected:
	uint8_t		WriteEventType = 0;
	uint8_t		ReadEventType = 0;
	const char**	PortABitNames = nullptr;
	const char**	PortBBitNames = nullptr;
	std::vector<FRegDisplayConfig>* RegConfig = nullptr;
};

class FCIA1Analysis : public FCIAAnalysis
{
public:
	FCIA1Analysis();
	const m6526_t* GetCIA() override;

};

class FCIA2Analysis : public FCIAAnalysis
{
public:
	FCIA2Analysis();
	const m6526_t* GetCIA() override;

};

void AddCIARegisterLabels(FC64Emulator* pEmulator);