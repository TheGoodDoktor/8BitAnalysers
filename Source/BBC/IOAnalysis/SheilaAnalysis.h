#pragma once

#include "IORegisterAnalysis.h"

class FBBCEmulator;

class FSheilaAnalysis : public FBBCIODevice
{
public:
	void	Init(FBBCEmulator* pEmulator);
	void	Reset();
};

void AddSheilaRegisterLabels(FBBCEmulator* pEmulator);
