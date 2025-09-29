#pragma once

#include "IORegisterAnalysis.h"

class FBBCEmulator;

class FJimAnalysis : public FBBCIODevice
{
public:
	void	Init(FBBCEmulator* pEmulator);
	void	Reset();
};

void AddJimRegisterLabels(FBBCEmulator* pEmulator);