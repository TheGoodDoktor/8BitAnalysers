#include "Debugger.h"
#include "SpectrumEmu.h"

void FDebugger::Init(FSpectrumEmu* pEmu)
{
	pEmulator = pEmu;
}

void FDebugger::CPUTick(uint64_t pins)
{

}

bool FDebugger::FrameTick(void)
{
	return bDebuggerStopped;
}

void	FDebugger::StepInto()
{

}

void	FDebugger::StepOver()
{

}

void	FDebugger::StepFrame()
{

}

void	FDebugger::StepScreenWrite()
{

}

bool FDebugger::IsAddressBreakpointed(FAddressRef addr)
{
	return false;
}
