#pragma once

#include <CodeAnalyser/CodeAnalyserTypes.h>

#include <vector>

class FSpectrumEmu;

enum class EDebugStepMode
{
	StepInto,
	StepOver,
	Frame,
	ScreenWrite
};

struct FBreakPoint
{
	FAddressRef		Address;
};

class FDebugger
{
public:
	void	Init(FSpectrumEmu* pEmu);
	void	CPUTick(uint64_t pins);
	bool	FrameTick(void);

	// Actions
	void	Break() { bDebuggerStopped = true; }
	void	Continue() { bDebuggerStopped = false; }
	void	StepInto();
	void	StepOver();
	void	StepFrame();
	void	StepScreenWrite();

	// Queries
	bool	Stopped() const { return bDebuggerStopped; }
	bool	IsAddressBreakpointed(FAddressRef addr);

	bool* GetDebuggerStoppedPtr() { return &bDebuggerStopped; }
private:
	FSpectrumEmu*	pEmulator = nullptr;
	bool			bDebuggerStopped = false;

	std::vector<FBreakPoint>	Breakpoints;
};

