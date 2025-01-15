#pragma once

#include "IORegisterAnalysis.h"

class FBBCEmulator;

class FFredAnalysis : public FBBCIODevice
{
public:
	void	Init(FBBCEmulator* pEmulator);
	void	Reset();
	void	OnRegisterRead(uint8_t reg, FAddressRef pc);
	void	OnRegisterWrite(uint8_t reg, uint8_t val, FAddressRef pc);

	void	DrawDetailsUI(void);
};

void AddFredRegisterLabels(FBBCEmulator* pEmulator);