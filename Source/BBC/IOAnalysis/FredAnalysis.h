#pragma once

#include "IORegisterAnalysis.h"

class FBBCEmulator;

class FFredAnalysis : public FBBCIODevice
{
public:
	void	Init(FBBCEmulator* pEmulator);
	void	Reset();
};

void AddFredRegisterLabels(FBBCEmulator* pEmulator);