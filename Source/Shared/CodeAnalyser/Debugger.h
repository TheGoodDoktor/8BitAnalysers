#pragma once

#include <CodeAnalyser/CodeAnalyserTypes.h>

#include <chips/z80.h>
#include <chips/m6502.h>
#include <vector>
#include <optional>

#include <stdio.h>

class FSpectrumEmu;
class FCodeAnalysisState;
struct FCodeAnalysisViewState;

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
	ScreenWrite,
	IORead,
	IOWrite,
	Interrupt,
	NMI,
};

// only add to end otherwise you'll break the file format
enum class EBreakpointType
{
	None,
	Exec,
	Data,
	Irq,
	NMI,
	In,
	Out,
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



/*struct FStackInfo
{
	FStackInfo(uint16_t basePtr) :BasePtr(basePtr) {}

	uint16_t					BasePtr;
	std::vector<FAddressRef>	SetBy;

	uint16_t				StackMin = 0xffff;
	uint16_t				StackMax = 0;
};*/


struct FEvent
{
	FEvent(uint8_t type, FAddressRef pc, uint16_t address, uint8_t value, uint16_t scanlinePos)
		: Type(type), PC(pc), Address(address), Value(value), ScanlinePos(scanlinePos) {}

	uint8_t			Type;
	uint16_t		Address;
	uint8_t			Value;
	uint16_t		ScanlinePos;
	FAddressRef		PC;
};

typedef void (*ShowEventInfoCB)(FCodeAnalysisState& state, const FEvent& event);


class FDebugger
{
public:
	void	Init(FCodeAnalysisState* pCodeAnalysis);
	void	CPUTick(uint64_t pins);
	int		OnInstructionExecuted(uint64_t pins);
	void	OnScanlineStart(int scanlineNo);
	void	OnMachineFrameStart();
	void	OnMachineFrameEnd();
	void	StartFrame();
	bool	FrameTick(void);

	void	LoadFromFile(FILE* fp);
	void	SaveToFile(FILE* fp);

	// Actions
	void	Break();
	void	Continue(std::optional<FAddressRef> stepToCursorAddr = std::nullopt);
	void	StepInto();
	void	StepOver();
	void	StepFrame();
	void	StepScreenWrite();
	void	StepIORead();
	void	StepIOWrite();
	void	SetPC(FAddressRef newPC) { PC = newPC; }

	// Breakpoints
	bool	AddExecBreakpoint(FAddressRef addr);
	bool	AddDataBreakpoint(FAddressRef addr, uint16_t size);
	bool	RemoveBreakpoint(FAddressRef addr);
	bool	ChangeBreakpointAddress(FAddressRef oldAddress,FAddressRef newAddress);
	const FBreakpoint* GetBreakpointForAddress(FAddressRef addr) const;
	FBreakpoint* GetBreakpointForAddress(FAddressRef addr) { return const_cast<FBreakpoint*>(const_cast<const FDebugger*>(this)->GetBreakpointForAddress(addr)); }
	void	SetScanlineBreakpoint(int scanline) { ScanlineBreakpoint = scanline;}
	void	ClearScanlineBreakpoint(void) { ScanlineBreakpoint = -1;}
	int		GetScanlineBreakpoint() const { return ScanlineBreakpoint;}
	
	// Watches
	void	AddWatch(FWatch watch);
	bool	RemoveWatch(FWatch watch);

	const std::vector<FWatch>& GetWatches() const { return Watches; }

	// Events 
	void RegisterEventType(uint8_t type, const char* pName, uint32_t col, ShowEventInfoCB pShowAddress = nullptr, ShowEventInfoCB pShowValue = nullptr);
	void ResetScanlineEvents(void);
	void RegisterEvent(uint8_t type, FAddressRef pc, uint16_t address, uint8_t value, uint16_t scanlinePos);
	const std::vector<FEvent>& GetEventTrace() const { return EventTrace; }
	const uint8_t* GetScanlineEvents() const { return ScanlineEvents; }
	uint32_t GetEventColour(uint8_t type);
	const char* GetEventName(uint8_t type);
	void ClearEvents();

	// Frame Trace
	const std::vector<FAddressRef>& GetFrameTrace() const { return FrameTrace; }
	bool	TraceForward(FCodeAnalysisViewState& viewState);
	bool	TraceBack(FCodeAnalysisViewState& viewState);

	// Stack
	void	RegisterNewStackPointer(uint16_t newSP, FAddressRef pc);
	bool	IsAddressOnStack(uint16_t address);

	std::vector<FCPUFunctionCall>& GetCallstack() { return CallStack; }

	// Queries
	bool	IsStopped() const { return bDebuggerStopped; }
	bool	IsAddressBreakpointed(FAddressRef addr) const;

	FAddressRef	GetPC() const { return PC; }
	const char*	GetRegisterStringValue(const char* regName) const;
	bool GetRegisterByteValue(const char* regName, uint8_t& outVal) const;
	bool GetRegisterWordValue(const char* regName, uint16_t& outVal) const;

	bool* GetDebuggerStoppedPtr() { return &bDebuggerStopped; }

	// UI
	void	DrawTrace(void);
	void	DrawCallStack(void);
	void	DrawStack(void);
	void	DrawWatches(void);
	void	DrawBreakpoints(void);
	void	DrawEvents(void);
	void	DrawUI(void);

	void FixupAddresRefs(void);
private:
	int		GetFrameTraceItemIndex(FAddressRef address);

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
	uint32_t					BreakpointMask = 0;
	std::optional<FAddressRef>	StepToCursorAddr;
	int							ScanlineBreakpoint = -1;
	std::vector<FWatch>			Watches;
	FWatch						SelectedWatch;
	std::vector<FAddressRef>	FrameTrace;
	int							FrameTraceItemIndex = -1;
	std::vector<FEvent>			EventTrace;
	int							SelectedEventIndex = -1;
	uint8_t						ScanlineEvents[320] = {0};
	bool						bClearEventsEveryFrame = true;
	bool						bWriteEventComments = false;

	//bool						bInterruptTriggered = false;

	std::vector<FCPUFunctionCall>	CallStack;
	int														SelectedCallstackNo = -1;

	std::vector<FAddressRef>	StackSetLocations;
	//std::vector<FStackInfo>		Stacks;
	int							CurrentStackNo = -1;

	uint16_t				StackMin = 0xffff;
	uint16_t				StackMax = 0;
};

void EventShowPixValue(FCodeAnalysisState& state, const FEvent& event);
void EventShowAttrValue(FCodeAnalysisState& state, const FEvent& event);
