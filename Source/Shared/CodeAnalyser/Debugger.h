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

struct FWatch : public FAddressRef
{
	FWatch() = default;
	FWatch(const FAddressRef addressRef) : FAddressRef(addressRef) {}
	FWatch(int16_t bankId, uint16_t address) : FAddressRef(bankId, address) {}
};

struct FCPUFunctionCall
{
	FAddressRef		FunctionAddr;
	FAddressRef		CallAddr;
	FAddressRef		ReturnAddr;
};

struct FStackInfo
{
	FStackInfo(uint16_t basePtr) :BasePtr(basePtr) {}

	uint16_t					BasePtr;
	std::vector<FAddressRef>	SetBy;

	uint16_t				StackMin = 0xffff;
	uint16_t				StackMax = 0;
};


class FDebugger
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	CPUTick(uint64_t pins);
	int		OnInstructionExecuted(uint64_t pins);
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
	void	SetPC(FAddressRef newPC) { PC = newPC; }

	// Breakpoints
	bool	AddExecBreakpoint(FAddressRef addr);
	bool	AddDataBreakpoint(FAddressRef addr, uint16_t size);
	bool	RemoveBreakpoint(FAddressRef addr);

	// Watches
	void	AddWatch(FWatch watch);
	bool	RemoveWatch(FWatch watch);

	const std::vector<FWatch>& GetWatches() const { return Watches; }

	// Frame Trace
	void	ResetFrameTrace() { FrameTrace.clear(); }
	const std::vector<FAddressRef>& GetFrameTrace() const { return FrameTrace; }

	// Stack
	void	RegisterNewStackPointer(uint16_t newSP, FAddressRef pc);
	bool	IsAddressOnStack(uint16_t address);

	std::vector<FCPUFunctionCall>& GetCallstack() { return CallStack; }

	// Queries
	bool	IsStopped() const { return bDebuggerStopped; }
	bool	IsAddressBreakpointed(FAddressRef addr) const;
	FAddressRef	GetPC() const { return PC; }

	bool* GetDebuggerStoppedPtr() { return &bDebuggerStopped; }

	// UI
	void	DrawTrace(void);
	void	DrawCallStack(void);
	void	DrawStack(void);
	void	DrawWatches(void);
	void	DrawBreakpoints(void);
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
	std::vector<FWatch>			Watches;
	FWatch						SelectedWatch;
	std::vector<FAddressRef>	FrameTrace;
	std::vector<FCPUFunctionCall>	CallStack;

	std::vector<FAddressRef>	StackSetLocations;
	std::vector<FStackInfo>		Stacks;
	int							CurrentStackNo = -1;

	uint16_t				StackMin;
	uint16_t				StackMax;


};

