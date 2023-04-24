#pragma once

#include <CodeAnalyser/CodeAnalyserTypes.h>

#include <vector>

class FSpectrumEmu;

enum class EDebugStepMode
{
	None,
	StepInto,
	StepOver,
	ToCursor,
	Frame,
	ScreenWrite
};

enum class EBreakpointType
{
	None,
	Exec,
	Data,
	Irq,
	NMI,
	In,
	Out
};

static const int kTrapId_None = 0;
static const int kTrapId_Step = 128;
static const int kTrapId_BpBase = kTrapId_Step + 1;

struct FBreakpoint
{
	FBreakpoint() {}
	FBreakpoint(FAddressRef addr, EBreakpointType type) :Address(addr), Type(type) {}
	FBreakpoint(FAddressRef addr, EBreakpointType type, uint16_t size) :Address(addr), Type(type), Size(size) {}

	FAddressRef		Address;
	int				Val = -1;
	EBreakpointType	Type = EBreakpointType::None;
	bool			bEnabled = true;
	uint16_t		Size = 1;
};

class FDebugger
{
public:
	void	Init(FSpectrumEmu* pEmu);
	void	CPUTick(uint64_t pins);
	bool	FrameTick(void);

	// Actions
	void	Break();
	void	Continue();
	void	StepInto();
	void	StepOver();
	void	StepFrame();
	void	StepScreenWrite();

	bool	AddExecBreakpoint(FAddressRef addr);
	bool	AddDataBreakpoint(FAddressRef addr, uint16_t size);
	bool	RemoveBreakpoint(FAddressRef addr);

	// Queries
	bool	Stopped() const { return bDebuggerStopped; }
	bool	IsAddressBreakpointed(FAddressRef addr);
	FAddressRef	GetPC() const { return PC; }

	bool* GetDebuggerStoppedPtr() { return &bDebuggerStopped; }
private:
	FSpectrumEmu*	pEmulator = nullptr;

	uint64_t		LastTickPins = 0;
	FAddressRef		PC;
	bool			bDebuggerStopped = false;
	EDebugStepMode	StepMode = EDebugStepMode::None;
	FAddressRef		StepOverPC;

	std::vector<FBreakpoint>	Breakpoints;
};

