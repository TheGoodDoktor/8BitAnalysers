#pragma once

#include <CodeAnalyser/CodeAnalyserTypes.h>

#include <chips/z80.h>
#include <chips/m6502.h>
#include <vector>

class FSpectrumEmu;
class FCodeAnalysisState;

// TODO: figure out how to forward dec these
//struct z80_t;
//struct m6502_t;

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
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	CPUTick(uint64_t pins);
	bool	FrameTick(void);

	void	LoadFromFile(FILE* fp);
	void	SaveToFile(FILE* fp);

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

	// UI
	void	DrawUI(void);
private:
	FCodeAnalysisState*	pCodeAnalysis = nullptr;

	ECPUType		CPUType = ECPUType::Unknown;
	z80_t*			pZ80 = nullptr;
	m6502_t*		pM6502 = nullptr;
	uint64_t		LastTickPins = 0;
	FAddressRef		PC;
	bool			bDebuggerStopped = false;
	EDebugStepMode	StepMode = EDebugStepMode::None;
	FAddressRef		StepOverPC;

	std::vector<FBreakpoint>	Breakpoints;
};

