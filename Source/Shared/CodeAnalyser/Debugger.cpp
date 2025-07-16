#include "Debugger.h"

#include <CodeAnalyser/CodeAnalyser.h>
#include <CodeAnalyser/MemoryAnalyser.h>

#include <chips/z80.h>

#include <imgui.h>
#include "UI/CodeAnalyserUI.h"
#include "Z80/Z80Disassembler.h"
#include "6502/M6502Disassembler.h"
#include <Util/GraphicsView.h>
#include "Misc/EmuBase.h"
#include <ImGuiSupport/ImGuiScaling.h>

static const uint32_t	BPMask_Exec			= 0x0001;
static const uint32_t	BPMask_DataWrite	= 0x0002;
static const uint32_t	BPMask_DataRead		= 0x0004;
static const uint32_t	BPMask_IORead		= 0x0008;
static const uint32_t	BPMask_IOWrite		= 0x0010;
static const uint32_t	BPMask_IRQ			= 0x0020;
static const uint32_t	BPMask_NMI			= 0x0040;

void FDebugger::Init(FCodeAnalysisState* pCA)
{
	pCodeAnalysis = pCA;
    CPUType = pCodeAnalysis->GetCPUInterface()->CPUType;

	switch (CPUType)
	{
		case ECPUType::Z80:
		   pZ80 = (z80_t*)pCodeAnalysis->GetCPUInterface()->GetCPUEmulator();
			StackMin = 0xffff;
			StackMax = 0;
			break;
		case ECPUType::M6502:
		case ECPUType::M65C02:	// hack for now - might be OK as the 6502 and M65C02 are very similar
			pM6502 = (m6502_t*)pCodeAnalysis->GetCPUInterface()->GetCPUEmulator();
			// Stack in 6502 is hard coded between 0x100-0x1ff
			StackMin = 0x1ff;
			StackMax = 0x1ff;
			break;
		default:
			assert(false && "Unknown CPU type");
			break;
	}
	

    Watches.clear();
	//Stacks.clear();
	Breakpoints.clear();
	CallStack.clear();
	FrameTrace.clear();
	FrameTraceItemIndex = -1;
}

void FDebugger::CPUTick(uint64_t pins)
{
    const uint64_t risingPins = pins & (pins ^ LastTickPins);
    int trapId = kTrapId_None;

    uint16_t addr = 0;
	bool bMemAccess = false;
	bool bWrite = false;
	bool bRead = false;
	bool bNewOp = false;
	bool bIORead = false;
	bool bIOWrite = false;
	bool bIrq = false;
	bool bNMI = false;
	uint32_t bpMaskCheck = 0;

    switch (CPUType)
    {
		case ECPUType::Z80:
			addr = Z80_GET_ADDR(pins);

			bMemAccess = !!((pins & Z80_CTRL_PIN_MASK) & Z80_MREQ);
			bWrite = (risingPins & Z80_CTRL_PIN_MASK) == (Z80_MREQ | Z80_WR);
			bRead = (risingPins & Z80_CTRL_PIN_MASK) == (Z80_MREQ | Z80_RD);
			bNewOp = z80_opdone(pZ80);
			bIORead = (pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_RD);
			bIOWrite = (pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_WR);
			bIrq = (pins & Z80_INT) && pZ80->iff1;
			bNMI = risingPins & Z80_NMI;
			break;
		case ECPUType::M6502:
		case ECPUType::M65C02:	// M65C02 is a superset of M6502
			addr = M6502_GET_ADDR(pins);
			bMemAccess = (pins & M6502_SYNC) == 0;
			bRead = pins & M6502_RW;
			bWrite = !bRead;
			bNewOp =  pins & M6502_SYNC;
			bIrq = risingPins & M6502_IRQ;
			bNMI = risingPins & M6502_NMI;
			break;
	}
	
    const FAddressRef addrRef = pCodeAnalysis->AddressRefFromPhysicalAddress(addr);

    if (bNewOp)
    {
        PC = pCodeAnalysis->AddressRefFromPhysicalAddress(pins & 0xffff);
		trapId = OnInstructionExecuted(pins);
	}

    // tick based stepping
    switch (StepMode)
    {
        // This is ZX Spectrum specific - need to think of a generic way of doing it - large memory breakpoint?
        case EDebugStepMode::ScreenWrite:
        {
            // break on screen memory write
            if (bWrite && pCodeAnalysis->pMemoryAnalyser->IsAddressInScreenMemory(addr))
                trapId = kTrapId_Step;            
        }
        break;

		case EDebugStepMode::IORead:
		{
			if (bIORead)
				trapId = kTrapId_Step;
		}
		break;

		case EDebugStepMode::IOWrite:
		{
			if (bIOWrite)
				trapId = kTrapId_Step;
		}
		break;

		case EDebugStepMode::Interrupt:
		{
			if (bIrq)
				trapId = kTrapId_Step;
		}
		break;

		case EDebugStepMode::NMI:
		{
		}
		break;
        
        default:
            break;
    }

	// only set the mask if we know we're on an address that's breakpointed
	if (bRead || bWrite)
	{
		FDataInfo* pDataInfo = pCodeAnalysis->GetDataInfoForAddress(addrRef);
		if (pDataInfo->bHasBreakpoint)
		{
			// setup breakpoint mask to check
			bpMaskCheck |= bWrite ? BPMask_DataWrite : 0;
			bpMaskCheck |= bRead ? BPMask_DataRead : 0;
		}
	}

    // iterate through data breakpoints
	// this can slow down if there are a lot of BPs
	// Do a mask check
	if (bpMaskCheck & BreakpointMask)
	{
		for (int i = 0; i < Breakpoints.size(); i++)
		{
			const FBreakpoint& bp = Breakpoints[i];

			if (bp.bEnabled)
			{
				switch (bp.Type)
				{
				case EBreakpointType::Data:
					if ((bWrite || bRead) &&
						addrRef.BankId == bp.Address.BankId &&
						addrRef.Address >= bp.Address.Address &&
						addrRef.Address < bp.Address.Address + bp.Size)
					{
						trapId = kTrapId_BpBase + i;
					}
					break;

				case EBreakpointType::Irq:
					if (bIrq)
						trapId = kTrapId_BpBase + i;
					break;

				case EBreakpointType::NMI:
					if (bNMI)
						trapId = kTrapId_BpBase + i;
					break;

					// In/Out - only for Z80
				case EBreakpointType::In:
					if (bIORead)
					{
						const uint16_t mask = bp.Val;
						if ((Z80_GET_ADDR(pins) & mask) == (bp.Address.Address & mask))
							trapId = kTrapId_BpBase + i;
					}
					break;

				case EBreakpointType::Out:
					if (bIOWrite)
					{
						const uint16_t mask = bp.Val;
						if ((Z80_GET_ADDR(pins) & mask) == (bp.Address.Address & mask))
							trapId = kTrapId_BpBase + i;
					}
					break;
                default:
                    break;
				}
			}
		}
	}

    if (trapId != kTrapId_None)
    {
        Break();
    }

    LastTickPins = pins;
}

int FDebugger::OnInstructionExecuted(uint64_t pins)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(PC);
	int trapId = kTrapId_None;

	if (StepToCursorAddr && PC == *StepToCursorAddr)
	{
		trapId = kTrapId_Step;
	}

	if (StepMode != EDebugStepMode::None)
	{
		switch (StepMode)
		{
		case EDebugStepMode::StepInto:
			trapId = kTrapId_Step;
			break;
		case EDebugStepMode::StepOver:
			// Check against step over PC value and stop
			if (PC == StepOverPC)
				trapId = kTrapId_Step;
			break;
        default:
            break;
		}
	}
	
	if(pCodeInfo && pCodeInfo->bHasBreakpoint)	// TODO: investigate why this can sometime be null (seen on C64)
	{
		for (int i = 0; i < Breakpoints.size(); i++)
		{
			const FBreakpoint& bp = Breakpoints[i];

			if (bp.bEnabled && 
				bp.Type == EBreakpointType::Exec &&
				PC == bp.Address)
					{
						trapId = kTrapId_BpBase + i;
						break;
					}
		}

		// TODO: remove breakpoint flag if no breakpoint found
	}

	// Handle IRQ
	const uint64_t risingPins = pins & (pins ^ LastTickPins);
	bool bIRQ = false;
	
	// Check for IRQ 
	switch (CPUType)
	{
	case ECPUType::Z80:
		bIRQ = (pins & Z80_INT) && pZ80->iff1;
		break;
	case ECPUType::M6502:
	case ECPUType::M65C02:	// M65C02 is a superset of M6502
		bIRQ = pM6502->brk_flags & M6502_BRK_IRQ;
	}

	if (bIRQ)
	{
		FCPUFunctionCall callInfo;
		callInfo.CallAddr = PC;
		switch (CPUType)
		{
			case ECPUType::Z80:
				callInfo.FunctionAddr = PC;	// Z80TODO: get interrupt handler address
				break;
			case ECPUType::M6502:
			case ECPUType::M65C02:
				callInfo.FunctionAddr = pCodeAnalysis->AddressRefFromPhysicalAddress(pCodeAnalysis->ReadWord(0xfffe));
				break;
		}

		callInfo.ReturnAddr = PC;
		CallStack.push_back(callInfo);
		//return UI_DBG_BP_BASE_TRAPID + 255;	//hack
	}

	FrameTrace.push_back(PC);

	// update stack size
	switch (CPUType) 
	{
		case ECPUType::Z80:
		{
		const uint16_t sp = pZ80->sp;
		if (sp == StackMin - 2 || StackMin == 0xffff)
			StackMin = sp;
		if (sp == StackMax + 2 || StackMax == 0)
			StackMax = sp;
		}
		break;

		case ECPUType::M6502:
		case ECPUType::M65C02:	// M65C02 is a superset of M6502
		{
			const uint16_t sp = pM6502->S + 0x100;
			StackMin = std::min(sp, StackMin);
			StackMax = 0x1ff;	// always starts here on 6502
		}
		break;
	}
	return trapId;
}

// called at the start of every scanline
void FDebugger::OnScanlineStart(int scanlineNo)
{
	//if (BreakpointMask & BPMask_Scanline)
	{
		// TODO: check for scanline breakpoint
		if(scanlineNo == ScanlineBreakpoint)
			Break();
	}
}

// called every machine frame
// will get called in the middle of emulation
void FDebugger::OnMachineFrameStart()
{
	if (bClearEventsEveryFrame)
		ClearEvents();

	ResetScanlineEvents();
}

void FDebugger::OnMachineFrameEnd()
{
	// handle frame stepping - should this be in the machine frame handler?
	if (StepMode == EDebugStepMode::Frame)
	{
		StepMode = EDebugStepMode::None;
		Break();
	}
}

void FDebugger::StartFrame() 
{ 
	FrameTrace.clear();
	FrameTraceItemIndex = -1;

	// Setup breakpoint mask 
	BreakpointMask = 0;

	for (int i = 0; i < Breakpoints.size(); i++)
	{
		const FBreakpoint& bp = Breakpoints[i];

		if (bp.bEnabled)
		{
			switch (bp.Type)
			{
			case EBreakpointType::Exec:
				BreakpointMask |= BPMask_Exec;
				break;
			case EBreakpointType::Data:
				BreakpointMask |= BPMask_DataWrite;
				BreakpointMask |= BPMask_DataRead;
				break;
			case EBreakpointType::In:
				BreakpointMask |= BPMask_IORead;
				break;
			case EBreakpointType::Out:
				BreakpointMask |= BPMask_IORead;
				break;
			case EBreakpointType::Irq:
				BreakpointMask |= BPMask_IRQ;
				break;
			case EBreakpointType::NMI:
				BreakpointMask |= BPMask_NMI;
				break;
            default:
                break;
			}
		}
	}
}

bool FDebugger::FrameTick(void)
{
	

	return bDebuggerStopped;
}

static const uint32_t kVersionNo = 4;

// Load state - breakpoints, watches etc.
void	FDebugger::LoadFromFile(FILE* fp)
{
	uint32_t versionNo = 0;
	fread(&versionNo, sizeof(uint32_t), 1, fp);

	// watches
	Watches.clear();
	uint32_t num = 0;
	fread(&num, sizeof(uint32_t), 1, fp);

	for (int i = 0; i < (int)num; i++)
	{
		FWatch& watch = Watches.emplace_back();
		fread(&watch.Val, sizeof(uint32_t), 1, fp);
	}

	// breakpoints
	Breakpoints.clear();
	fread(&num, sizeof(uint32_t), 1, fp);
	for (int i = 0; i < (int)num; i++)
	{
		FBreakpoint& bp = Breakpoints.emplace_back();
		fread(&bp.Address.Val, sizeof(uint32_t), 1, fp);	// address
		fread(&bp.bEnabled, sizeof(bp.bEnabled), 1, fp);	// enabled
		fread(&bp.Type, sizeof(bp.Type), 1, fp);	// Type
		fread(&bp.Size, sizeof(bp.Size), 1, fp);	// Size
		fread(&bp.Val, sizeof(bp.Val), 1, fp);		// Val

		// make sure code info is flagged
		if(bp.Type == EBreakpointType::Exec)
		{ 
			FCodeAnalysisState& state = *pCodeAnalysis;
			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(bp.Address);
			if(pCodeInfo != nullptr)
				pCodeInfo->bHasBreakpoint = true;
		}
		else if (bp.Type == EBreakpointType::Data)
		{
			FAddressRef bpAddr = bp.Address;
			for (int i = 0; i < bp.Size; i++)
			{
				FDataInfo* dataInfo = pCodeAnalysis->GetDataInfoForAddress(bpAddr);
				dataInfo->bHasBreakpoint = true;
				pCodeAnalysis->AdvanceAddressRef(bpAddr);
			}
		}
	}

	// frame trace
	if (versionNo > 1)
	{
		FrameTrace.clear();
		FrameTraceItemIndex = -1;
		fread(&num, sizeof(uint32_t), 1, fp);
		for (int i = 0; i < (int)num; i++)
		{
			FAddressRef& address = FrameTrace.emplace_back();
			fread(&address.Val, sizeof(uint32_t), 1, fp);	// address
		}
	}

	// PC
	if (versionNo > 2)
		fread(&PC.Val, sizeof(uint32_t), 1, fp);	

	if (versionNo > 3)
		fread(&ScanlineBreakpoint, sizeof(int), 1, fp);
}

// Save state - breakpoints, watches etc.
void	FDebugger::SaveToFile(FILE* fp)
{
	fwrite(&kVersionNo, sizeof(uint32_t), 1, fp);

	// watches
	uint32_t num = (uint32_t)Watches.size();
	fwrite(&num, sizeof(uint32_t), 1, fp);

	for (int i = 0; i < (int)num; i++)
	{
		const FWatch& watch = Watches[i];
		fwrite(&watch.Val, sizeof(uint32_t), 1, fp);
	}

	// breakpoints
	num = (uint32_t)Breakpoints.size();
	fwrite(&num, sizeof(uint32_t), 1, fp);
	for (int i = 0; i < (int)num; i++)
	{
		const FBreakpoint& bp = Breakpoints[i];
		fwrite(&bp.Address.Val, sizeof(uint32_t), 1, fp);	// address
		fwrite(&bp.bEnabled, sizeof(bp.bEnabled), 1, fp);	// enabled
		fwrite(&bp.Type, sizeof(bp.Type), 1, fp);	// Type
		fwrite(&bp.Size, sizeof(bp.Size), 1, fp);	// Size
		fwrite(&bp.Val, sizeof(bp.Val), 1, fp);		// Val
	}

	// frame trace
	num = (uint32_t)FrameTrace.size();
	fwrite(&num, sizeof(uint32_t), 1, fp);
	for (int i = 0; i < (int)num; i++)
	{
		fwrite(&FrameTrace[i].Val,sizeof(uint32_t), 1, fp);	// address
	}

	// PC
	fwrite(&PC.Val,sizeof(uint32_t), 1, fp);

	// Scanline BP
	fwrite(&ScanlineBreakpoint, sizeof(int), 1, fp);
}


void FDebugger::Break()
{ 
    StepToCursorAddr = std::nullopt;
    StepMode = EDebugStepMode::None;
    bDebuggerStopped = true;
}

void FDebugger::Continue(std::optional<FAddressRef> stepToCursorAddr)
{ 
    StepToCursorAddr = stepToCursorAddr;
    StepMode = EDebugStepMode::None; 
    bDebuggerStopped = false; 

    SelectedCallstackNo = -1;
}


void FDebugger::StepInto()
{
    StepMode = EDebugStepMode::StepInto;
    bDebuggerStopped = false;
}

// check if the an instruction is a 'step over' op 
static bool IsStepOverOpcode(ECPUType cpuType, const std::vector<uint8_t>& opcodes)
{
    if (cpuType == ECPUType::Z80)
    {
        switch (opcodes[0])
        {
            // CALL nnnn 
        case 0xCD:
            // CALL cc,nnnn 
        case 0xDC: case 0xFC: case 0xD4: case 0xC4:
        case 0xF4: case 0xEC: case 0xE4: case 0xCC:
            // DJNZ d 
        case 0x10:
			// HALT
		case 0x76:
			return true;
		// TODO: LDIR & others
		case 0xED:
		{
			switch (opcodes[1])
			{
				case 0xB0:	// LDIR
					return true;
				default:
					return false;
			}
		}
        default:
            return false;
        }
    }
    else if (cpuType == ECPUType::M6502 || cpuType == ECPUType::M65C02)
    {
        // on 6502, only JSR qualifies 
        return opcodes[0] == 0x20;
    }
    else
    {
        return false;
    }
}

void	FDebugger::StepOver()
{
	std::vector<uint8_t> stepOpcodes;
   
    bDebuggerStopped = false;
    uint16_t nextPC = 0;
	switch (CPUType)
	{
		case ECPUType::Z80:
			nextPC = Z80DisassembleGetNextPC(PC.Address, *pCodeAnalysis, stepOpcodes);
			break;
		case ECPUType::M6502:
		case ECPUType::M65C02:
			nextPC = M6502DisassembleGetNextPC(PC.Address, *pCodeAnalysis, stepOpcodes);
			break;
	}

    if (IsStepOverOpcode(CPUType, stepOpcodes))
    {
        StepMode = EDebugStepMode::StepOver;
        StepOverPC = pCodeAnalysis->AddressRefFromPhysicalAddress(nextPC);
    }
    else 
    {
        StepMode = EDebugStepMode::StepInto;
    }
}

void	FDebugger::StepFrame()
{
	StepMode = EDebugStepMode::Frame;
    bDebuggerStopped = false;
}

void	FDebugger::StepScreenWrite()
{
    StepMode = EDebugStepMode::ScreenWrite;
    bDebuggerStopped = false;
}

void	FDebugger::StepIORead()
{
	StepMode = EDebugStepMode::IORead;
	bDebuggerStopped = false;
}

void	FDebugger::StepIOWrite()
{
	StepMode = EDebugStepMode::IOWrite;
	bDebuggerStopped = false;
}

// Breakpoints

bool FDebugger::AddExecBreakpoint(FAddressRef addr)
{
	if (IsAddressBreakpointed(addr))
		return false;

	FCodeInfo* pCodeInfo = pCodeAnalysis->GetCodeInfoForAddress(addr);
	assert(pCodeInfo);
	pCodeInfo->bHasBreakpoint = true;
	Breakpoints.emplace_back(addr, EBreakpointType::Exec);
	return true;
}

bool FDebugger::AddDataBreakpoint(FAddressRef addr, uint16_t size)
{
	if (IsAddressBreakpointed(addr))
		return false;

	FAddressRef bpAddr = addr;
	for (int i = 0; i < size; i++)
	{
		FDataInfo* dataInfo = pCodeAnalysis->GetDataInfoForAddress(bpAddr);
		dataInfo->bHasBreakpoint = true;
		pCodeAnalysis->AdvanceAddressRef(bpAddr);
	}

	Breakpoints.emplace_back(addr, EBreakpointType::Data,size);
	return true;
}

bool FDebugger::RemoveBreakpoint(FAddressRef addr)
{
	for (int i = 0; i < Breakpoints.size(); i++)
	{
		FBreakpoint& bp = Breakpoints[i];

		if (bp.Address == addr)
		{
			if (bp.Type == EBreakpointType::Exec)
			{ 
				FCodeInfo* pCodeInfo = pCodeAnalysis->GetCodeInfoForAddress(addr);
				assert(pCodeInfo);
				pCodeInfo->bHasBreakpoint = false;
			}
			else if (bp.Type == EBreakpointType::Data)
			{
				FAddressRef bpAddr = addr;
				for (int i = 0; i < bp.Size; i++)
				{
					FDataInfo* dataInfo = pCodeAnalysis->GetDataInfoForAddress(bpAddr);
					dataInfo->bHasBreakpoint = false;
					pCodeAnalysis->AdvanceAddressRef(bpAddr);
				}
			}
			Breakpoints[i] = Breakpoints.back();
			Breakpoints.pop_back();
			
			return true;
		}
	}
	return false;
}

bool FDebugger::ChangeBreakpointAddress(FAddressRef oldAddress, FAddressRef newAddress)
{
	FBreakpoint* pBP = GetBreakpointForAddress(oldAddress);
	if(pBP == nullptr || IsAddressBreakpointed(newAddress))	// return false if either address is invalid
		return false;
	pBP->Address = newAddress;
	return true;
}


const FBreakpoint* FDebugger::GetBreakpointForAddress(FAddressRef addr) const
{
	for (int i = 0; i < Breakpoints.size(); i++)
	{
		if (Breakpoints[i].Address == addr)
			return &Breakpoints[i];
	}

	return nullptr;
}


bool FDebugger::IsAddressBreakpointed(FAddressRef addr) const
{
	return GetBreakpointForAddress(addr) != nullptr;
}

// Watches

void FDebugger::AddWatch(FWatch watch)
{
	Watches.push_back(watch);
}

bool FDebugger::RemoveWatch(FWatch watch)
{
	for (auto watchIt = Watches.begin(); watchIt != Watches.end();)
	{
		if (*watchIt == watch)
			watchIt = Watches.erase(watchIt);
		else
			++watchIt;
	}

	return true;
}

// Stack

void FDebugger::RegisterNewStackPointer(uint16_t newSP, FAddressRef pc)
{
	if (pc.IsValid())
	{
		bool bFound = false;
		for (int i = 0; i < StackSetLocations.size(); i++)
		{
			if (StackSetLocations[i] == pc)
			{
				bFound = true;
				break;
			}
		}

		if(bFound == false)
			StackSetLocations.push_back(pc);
	}
	/*CurrentStackNo = -1;

	for (int i = 0; i < Stacks.size(); i++)
	{
		if (Stacks[i].BasePtr == newSP)
		{
			CurrentStackNo = i;
			break;
		}
	}

	if (CurrentStackNo == -1)
	{
		CurrentStackNo = (int)Stacks.size();
		Stacks.push_back(FStackInfo(newSP));
	}

	// maybe this needs to be somewhere else?
	if (pc.IsValid())
	{
		FStackInfo& stack = Stacks[CurrentStackNo];
		for (int i = 0; i < stack.SetBy.size(); i++)
		{
			if (stack.SetBy[i] == pc)
				return;
		}

		stack.SetBy.push_back(pc);
	}*/
}

bool FDebugger::IsAddressOnStack(uint16_t address) 
{ 
	//FStackInfo& stack = Stacks[CurrentStackNo];

	return address >= StackMin && address <= StackMax;
}

// Events 
void FDebugger::ResetScanlineEvents(void)
{
	memset(ScanlineEvents, 0, sizeof(ScanlineEvents));
}

static const size_t kEventNameLength = 32;
struct FEventTypeInfo
{
	char		EventName[kEventNameLength];
	uint32_t	EventColour;

	ShowEventInfoCB	ShowAddressCB = nullptr;
	ShowEventInfoCB	ShowValueCB = nullptr;
	
	bool bEnabled = true;
};

std::vector<FEventTypeInfo> g_EventTypeInfo;

void FDebugger::RegisterEventType(uint8_t type, const char* pName, uint32_t col, ShowEventInfoCB pShowAddress, ShowEventInfoCB pShowValue)
{
	std::vector<FEventTypeInfo>& eventTypeInfo = g_EventTypeInfo;

	if(type >= eventTypeInfo.size())
		eventTypeInfo.resize(type + 1);

	FEventTypeInfo& typeInfo = eventTypeInfo[type];
	assert(strlen(pName) < kEventNameLength);
	strncpy(typeInfo.EventName, pName, kEventNameLength);
	typeInfo.EventColour = col;
	typeInfo.ShowAddressCB = pShowAddress;
	typeInfo.ShowValueCB = pShowValue;
}

void FDebugger::RegisterEvent(uint8_t type, FAddressRef pc, uint16_t address, uint8_t value, uint16_t scanlinePos)
{
	std::vector<FEventTypeInfo>& eventTypeInfo = g_EventTypeInfo;

	if (!eventTypeInfo[type].bEnabled)
		return;

	ScanlineEvents[scanlinePos] = type;
	EventTrace.emplace_back(type, pc, address, value, scanlinePos);

	if(bWriteEventComments)
	{ 
		FCodeAnalysisState& state = *pCodeAnalysis;
		FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(pc);

		if (pCodeInfo != nullptr && pCodeInfo->Comment.empty())
			pCodeInfo->Comment = GetEventName(type);
	}
}

uint32_t FDebugger::GetEventColour(uint8_t type)
{
	return g_EventTypeInfo[type].EventColour;
}

const char* FDebugger::GetEventName(uint8_t type)
{
	return g_EventTypeInfo[type].EventName;
}

void FDebugger::ClearEvents()
{
	EventTrace.clear();
}

bool	FDebugger::TraceForward(FCodeAnalysisViewState& viewState)
{
	const FCodeAnalysisItem& cursorItem = viewState.GetCursorItem();

	if (FrameTraceItemIndex == -1 || FrameTrace[FrameTraceItemIndex] != cursorItem.AddressRef)
		FrameTraceItemIndex = GetFrameTraceItemIndex(cursorItem.AddressRef);

	if (FrameTraceItemIndex >= 0 && FrameTraceItemIndex < FrameTrace.size() - 1)
	{
		FrameTraceItemIndex++;
		viewState.GoToAddress(FrameTrace[FrameTraceItemIndex]);
	}
	return FrameTraceItemIndex != -1;
}

bool	FDebugger::TraceBack(FCodeAnalysisViewState& viewState)
{
	const FCodeAnalysisItem& cursorItem = viewState.GetCursorItem();

	if (FrameTraceItemIndex == -1 || FrameTrace[FrameTraceItemIndex] != cursorItem.AddressRef)
		FrameTraceItemIndex = GetFrameTraceItemIndex(cursorItem.AddressRef);

	if (FrameTraceItemIndex > 0)
	{
		FrameTraceItemIndex--;
		viewState.GoToAddress(FrameTrace[FrameTraceItemIndex]);
	}

	return FrameTraceItemIndex != -1;
}

bool FDebugger::GetRegisterByteValue(const char* regName, uint8_t& outVal) const
{
	if (CPUType == ECPUType::Z80)
	{
		if (strcmp(regName, "A") == 0)
			return outVal = pZ80->a, true;
		else if (strcmp(regName, "F") == 0)
			return outVal = pZ80->f, true;
		else if (strcmp(regName, "B") == 0)
			return outVal = pZ80->b, true;
		else if (strcmp(regName, "C") == 0)
			return outVal = pZ80->c, true;
		else if (strcmp(regName, "D") == 0)
			return outVal = pZ80->d, true;
		else if (strcmp(regName, "E") == 0)
			return outVal = pZ80->e, true;
		else if (strcmp(regName, "H") == 0)
			return outVal = pZ80->h, true;
		else if (strcmp(regName, "L") == 0)
			return outVal = pZ80->l, true;
		else if (strcmp(regName, "R") == 0)
			return outVal = pZ80->r, true;
		else if (strcmp(regName, "I") == 0)
			return outVal = pZ80->i, true;
	}
	else if (CPUType == ECPUType::M6502 || CPUType == ECPUType::M65C02)
	{
		if (strcmp(regName, "A") == 0)
			return outVal = pM6502->A, true;
		else if (strcmp(regName, "X") == 0)
			return outVal = pM6502->X, true;
		else if (strcmp(regName, "Y") == 0)
			return outVal = pM6502->Y, true;
		else if (strcmp(regName, "S") == 0)
			return outVal = pM6502->S, true;
		else if (strcmp(regName, "P") == 0)
			return outVal = pM6502->P, true;
	}

	return false;
}

bool FDebugger::GetRegisterWordValue(const char* regName, uint16_t& outVal) const
{
	if (CPUType == ECPUType::Z80)
	{
		if (strcmp(regName, "AF") == 0)
			return outVal = pZ80->af, true;
		else if (strcmp(regName, "BC") == 0)
			return outVal = pZ80->bc, true;
		else if (strcmp(regName, "DE") == 0)
			return outVal = pZ80->de, true;
		else if (strcmp(regName, "HL") == 0)
			return outVal = pZ80->hl, true;
		else if (strcmp(regName, "IX") == 0)
			return outVal = pZ80->ix, true;
		else if (strcmp(regName, "IY") == 0)
			return outVal = pZ80->iy, true;
		else if (strcmp(regName, "SP") == 0)
			return outVal = pZ80->sp, true;
		else if (strcmp(regName, "PC") == 0)
			return outVal = pZ80->pc, true;
	}
	else if (CPUType == ECPUType::M6502 || CPUType == ECPUType::M65C02)
	{
		if (strcmp(regName, "PC") == 0)
			return outVal = pM6502->PC, true;
	}
	return false;
}

const char* FDebugger::GetRegisterStringValue(const char* regName) const
{
	uint8_t byteVal = 0;
	if(GetRegisterByteValue(regName,byteVal))
		return NumStr(byteVal);
	uint16_t wordVal = 0;
	if (GetRegisterWordValue(regName, wordVal))
		return NumStr(wordVal);
		
	return "TODO";
}


// UI Code

int FDebugger::GetFrameTraceItemIndex(FAddressRef address)
{
	for (int i = 0; i < FrameTrace.size(); i++)
	{
		if (FrameTrace[i] == address)
			return i;
	}

	return -1;
}

void FDebugger::DrawTrace(void)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	//const float line_height = ImGui::GetTextLineHeight();
	
	if (ImGui::Button("Trace Back"))
	{
		TraceBack(viewState);
	}
	ImGui::SameLine();
	if (ImGui::Button("Trace Forward"))
	{
		TraceForward(viewState);
	}

	if (ImGui::BeginChild("TraceListChild"))
	{
		ImGuiListClipper clipper;
		clipper.Begin((int)FrameTrace.size());
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FAddressRef codeAddress = FrameTrace[FrameTrace.size() - i - 1];
				FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(codeAddress);
				DrawCodeAddress(state, viewState, codeAddress, false);	// draw current PC
			}
		}
	}
	ImGui::EndChild();
}

void FDebugger::DrawCallStack(void)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	if (ImGui::Selectable("##callstacktop", SelectedCallstackNo == CallStack.size()))
	{
		viewState.GoToAddress(CallStack.empty() ? state.CPUInterface->GetPC() : CallStack.back().FunctionAddr, false);
		SelectedCallstackNo = (int)CallStack.size();
	}

	// Draw current function & PC position
	if (CallStack.empty() == false)
	{
		const FLabelInfo* pLabel = state.GetLabelForAddress(CallStack.back().FunctionAddr);
		if (pLabel != nullptr)
		{
			ImGui::SameLine();
			ImGui::Text("%s :", pLabel->GetName());
		}
	}

	ImGui::SameLine();
	DrawCodeAddress(state, viewState, state.CPUInterface->GetPC(), false);	// draw current PC

	for (int i = (int)CallStack.size() - 1; i >= 0; i--)
	{
		ImGui::PushID(i);
		if (ImGui::Selectable("##callstack", SelectedCallstackNo == i))
		{
			viewState.GoToAddress(CallStack[i].CallAddr, false);
			SelectedCallstackNo = i;
		}
		ImGui::PopID();

		if (i > 0)
		{
			const FLabelInfo* pLabel = state.GetLabelForAddress(CallStack[i - 1].FunctionAddr);
			if (pLabel != nullptr)
			{
				ImGui::SameLine();
				ImGui::Text("%s :", pLabel->GetName());
			}
		}
		
		ImGui::SameLine();
		DrawCodeAddress(state, viewState, CallStack[i].CallAddr, false);
	}
}

void FDebugger::DrawStack(void)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const uint16_t sp = state.CPUInterface->GetSP();

	/*for (int i = 0; i < Stacks.size(); i++)
	{
		FStackInfo& stack = Stacks[i];

		ImGui::Text("Stack %d", i);
		DrawAddressLabel(state, viewState, stack.BasePtr);
		for (int j = 0; j < stack.SetBy.size(); j++)
		{
			DrawAddressLabel(state, viewState, stack.SetBy[j]);

		}
	}*/

	if (ImGui::CollapsingHeader("Stack Set Locations"))
	{
		for (int i = 0; i < StackSetLocations.size(); i++)
		{
			ImGui::Text("%d: ",i);
			DrawAddressLabel(state, viewState, StackSetLocations[i]);
		}
	}

	if (StackMin >= StackMax)	// stack is invalid
	{
		ImGui::Text("No valid stack discovered");
		return;
	}

	if (sp < StackMin || sp > StackMax)	// sp is not in range
	{
		ImGui::Text("Stack pointer: %s", NumStr(sp));
		DrawAddressLabel(state, state.GetFocussedViewState(), sp);
		ImGui::SameLine();
		ImGui::Text("not in stack range(%s - %s)", NumStr(StackMin), NumStr(StackMax));
		return;
	}

	// StackInfo
	if (StackMax > StackMin)
	{
		//ImGui::SameLine();
		ImGui::Text("Stack range: ");
		DrawAddressLabel(state, viewState, StackMin);
		ImGui::SameLine();
		DrawAddressLabel(state, viewState, StackMax);
	}

	//static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

	if (ImGui::BeginTable("stackinfo", 4, flags))
	{
		ImGui::TableSetupColumn("Address");
		ImGui::TableSetupColumn("Value");
		ImGui::TableSetupColumn("Comment");
		ImGui::TableSetupColumn("Set by");
		ImGui::TableHeadersRow();

		for (int stackAddr = sp; stackAddr <= StackMax; stackAddr += 2)
		{
			ImGui::TableNextRow();

			uint16_t stackVal = state.ReadWord(stackAddr);
			FDataInfo* pDataInfo = state.GetWriteDataInfoForAddress(stackAddr);
			const FAddressRef writerAddr = state.GetLastWriterForAddress(stackAddr);

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", NumStr((uint16_t)stackAddr));

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s :", NumStr(stackVal));
			DrawAddressLabel(state, viewState, stackVal);

			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%s", pDataInfo->Comment.c_str());

			ImGui::TableSetColumnIndex(3);
			if (writerAddr.IsValid())
			{
				ImGui::Text("%s :", NumStr(writerAddr.Address));
				DrawAddressLabel(state, viewState, writerAddr);
			}
			else
			{
				ImGui::Text("None");
			}
		}

		ImGui::EndTable();
	}
}


void DrawRegisters_Z80(FCodeAnalysisState& state);
void DrawRegisters_6502(FCodeAnalysisState& state);

void DrawRegisters(FCodeAnalysisState& state)
{
	switch (state.CPUInterface->CPUType)
	{ 
		case ECPUType::Z80:
			DrawRegisters_Z80(state);
			break;
		case ECPUType::M6502:
		case ECPUType::M65C02:
			DrawRegisters_6502(state);
			break;
	}
}

void FDebugger::DrawWatches(void)
{
    FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	bool bDeleteSelectedWatch = false;

	for (const auto& watch : Watches)
	{
		FDataInfo* pDataInfo = state.GetDataInfoForAddress(watch);
		ImGui::PushID(watch.Val);
		if (ImGui::Selectable("##watchselect", watch == SelectedWatch, 0))
		{
			SelectedWatch = watch;
		}
		if (SelectedWatch.IsValid() && ImGui::BeginPopupContextItem("watch context menu"))
		{
			if (ImGui::Selectable("Delete Watch"))
			{
				bDeleteSelectedWatch = true;
			}
			if (ImGui::Selectable("Toggle Breakpoint"))
			{
				FDataInfo* pInfo = state.GetDataInfoForAddress(SelectedWatch);
				state.ToggleDataBreakpointAtAddress(SelectedWatch, pInfo->ByteSize);
			}

			ImGui::EndPopup();
		}
		ImGui::SetItemAllowOverlap();	// allow buttons
		ImGui::SameLine();
		DrawDataInfo(state, viewState, FCodeAnalysisItem(pDataInfo, watch.BankId, watch.Address), true, state.bAllowEditing);

		// TODO: Edit Watch
		ImGui::PopID();
	}

	if (bDeleteSelectedWatch)
		RemoveWatch(SelectedWatch);
}

const char* GetBreakpointTypeText(EBreakpointType type)
{
	switch (type)
	{
	case EBreakpointType::Exec:
		return "Exec";
	case EBreakpointType::Data:
		return "Data";
    default:
        return "Unknown";
	}
}

void FDebugger::DrawBreakpoints(void)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable("Breakpoints", 4, flags))
	{
		const float charWidth = ImGui_GetFontCharWidth();
		ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 10 * charWidth);
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,6 * charWidth);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed,6 * charWidth);
		ImGui::TableHeadersRow();
		FAddressRef deleteRef;
		for (auto& bp : Breakpoints)
		{
			ImGui::PushID(bp.Address.Val);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Checkbox("##Enabled", &bp.bEnabled);
			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				deleteRef = bp.Address;
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s:", NumStr(bp.Address.Address));
			DrawAddressLabel(state, viewState, bp.Address);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%s", GetBreakpointTypeText(bp.Type));
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%d", bp.Size);
			ImGui::PopID();
		}

		if (deleteRef.IsValid())
		{
			RemoveBreakpoint(deleteRef);
		}
		ImGui::EndTable();
	}
}

// Generic Event render functions
void EventShowPixValue(FCodeAnalysisState& state, const FEvent& event)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height / 2;
	pos.y += line_height / 4;
	const uint8_t pixels = event.Value;

	for (int i = 0; i < 8; i++)
	{
		if (pixels & (1 << (7 - i)))
			dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);

		dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);
		pos.x += rectSize;
	}
}

void EventShowAttrValue(FCodeAnalysisState& state, const FEvent& event)
{
	const uint32_t* colourLUT = state.Config.CharacterColourLUT;

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float line_height = ImGui::GetTextLineHeight();
	const float rectSize = line_height;

	const uint8_t colAttr = event.Value;

	const bool bBright = !!(colAttr & (1 << 6));
	const uint32_t inkCol = GetColFromAttr(colAttr & 7, colourLUT, bBright);
	const uint32_t paperCol = GetColFromAttr(colAttr >> 3, colourLUT, bBright);

	// Ink
	{
		const ImVec2 rectMin(pos.x, pos.y);
		const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize / 2);
		dl->AddRectFilled(rectMin, rectMax, inkCol);
	}

	// Paper
	{
		const ImVec2 rectMin(pos.x, pos.y + rectSize / 2);
		const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
		dl->AddRectFilled(rectMin, rectMax, paperCol);
	}

	dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);
	// tool tip?
	//ImGui::Text("%s", NumStr(event.Value));
}
void FDebugger::DrawEvents(void)
{
	std::vector<FEventTypeInfo>& eventTypeInfo = g_EventTypeInfo;
	FCodeAnalysisState& state = *pCodeAnalysis;
	FEmuBase* pEmuBase = state.GetEmulator();

	if (ImGui::Button("Clear"))
		ClearEvents();
	ImGui::SameLine();
	ImGui::Checkbox("Clear Every Frame", &bClearEventsEveryFrame);
	ImGui::SameLine();
	if (ImGui::Button("Write Comments"))
	{
		for (const auto& event : EventTrace)
		{
			FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(event.PC);
			if(pCodeInfo != nullptr && pCodeInfo->Comment.empty())
				pCodeInfo->Comment = GetEventName(event.Type);
		}
	}
	//disabled for now as it's a bit dangerous
	//ImGui::SameLine();
	//ImGui::Checkbox("Write On Register", &bWriteEventComments);

	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const float lineHeight = ImGui::GetTextLineHeight();
	
	const float rectSize = lineHeight;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	
	if (ImGui::TreeNode("Event Types"))
	{
		const bool bSelectAll = ImGui::Button("Select All");
		ImGui::SameLine();
		const bool bSelectNone = ImGui::Button("Select None");

		if (bSelectAll || bSelectNone)
		{
			const bool writeVal = bSelectAll ? true : false;
			for(auto& eventType : eventTypeInfo)
				eventType.bEnabled = writeVal;
		}
				
		for (auto& eventType : eventTypeInfo)
		{
			if (eventType.EventName[0] == 0)	// Skip 'None'
				continue;
			ImVec2 pos = ImGui::GetCursorScreenPos();
			const ImVec2 rectMin(pos.x, pos.y + 3);
			const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize + 3);
			dl->AddRectFilled(rectMin, rectMax, eventType.EventColour);

			ImGui::Text("  "); 
			ImGui::SameLine();
			ImGui::Checkbox(eventType.EventName, &eventType.bEnabled);
		}
		ImGui::TreePop();
	}

	viewState.HighlightScanline = -1;

	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
	if (ImGui::BeginTable("Events", 5, flags))
	{
		const float fontSize = ImGui::GetFontSize();

		ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
		ImGui::TableSetupColumn("Scanline", ImGuiTableColumnFlags_WidthFixed, fontSize * 3);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, fontSize * 14);
		ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, fontSize * 8);
		ImGui::TableHeadersRow();
		ImGuiListClipper clipper;
		clipper.Begin((int)EventTrace.size());
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FEvent& event = EventTrace[i];
				const FEventTypeInfo& typeInfo = g_EventTypeInfo[event.Type];
				ImGui::PushID(i);
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				const bool bSelected = SelectedEventIndex == i;
				ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
				if (ImGui::Selectable("##eventselect", bSelected, selectableFlags, ImVec2(0, 0)))
				{
					SelectedEventIndex = i;
				}
				ImGui::SetItemAllowOverlap();	// allow buttons
				ImGui::SameLine();
				ImGui::Text("%d", event.ScanlinePos);
				
				// highlight scanline
				if (bSelected)
				{
					const uint32_t col = GetEventColour(event.Type);
					pEmuBase->SetScanlineHighlight(event.ScanlinePos, col);
				}
				ImGui::TableSetColumnIndex(1);
				ImVec2 pos = ImGui::GetCursorScreenPos();
					
				// Type
				const ImVec2 rectMin(pos.x, pos.y);
				const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
				dl->AddRectFilled(rectMin, rectMax, typeInfo.EventColour);

				ImGui::Text("   %s", GetEventName(event.Type));
					
				if (ImGui::IsItemHovered())
				{
					viewState.HighlightScanline = event.ScanlinePos;
				}

				// PC
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s:", NumStr(event.PC.Address));
				DrawAddressLabel(state, viewState, event.PC);

				// Address
				ImGui::TableSetColumnIndex(3);
				if (typeInfo.ShowAddressCB != nullptr)
				{
					typeInfo.ShowAddressCB(state, event);
				}
				else
				{
					ImGui::Text("%s:", NumStr(event.Address));
					DrawAddressLabel(state, viewState, event.Address);
				}

				// Value
				ImGui::TableSetColumnIndex(4);
				if (typeInfo.ShowValueCB != nullptr)
				{
					typeInfo.ShowValueCB(state, event);
				}
				else
				{
					ImGui::Text("%s", NumStr(event.Value));
				}
				ImGui::PopID();
			}
		}

		ImGui::EndTable();

	}
}

#define DOCKABLE_DEBUGGER 1

void FDebugger::DrawUI(void)
{
	/*if (ImGui::Button("Step IO Read"))
		StepIORead();
	ImGui::SameLine();
	if (ImGui::Button("Step IO Write"))
		StepIOWrite();
		*/
#if DOCKABLE_DEBUGGER
	if (ImGui::Begin("Breakpoints"))
	{
		DrawBreakpoints();
	}
	ImGui::End();

	if (ImGui::Begin("Watches"))
	{
		DrawWatches();
	}
	ImGui::End();

	if (ImGui::Begin("Registers"))
	{
		DrawRegisters(*pCodeAnalysis);
	}
	ImGui::End();

	if (ImGui::Begin("Stack"))
	{
		DrawStack();
	}
	ImGui::End();

	if (ImGui::Begin("Call Stack"))
	{
		DrawCallStack();
	}
	ImGui::End();

	if (ImGui::Begin("Trace"))
	{
		DrawTrace();
	}
	ImGui::End();

	if (ImGui::Begin("Events"))
	{
		DrawEvents();
	}
	ImGui::End();
#else
    if (ImGui::BeginTabBar("DebuggerTabBar"))
    {
        if (ImGui::BeginTabItem("Breakpoints"))
        {
			DrawBreakpoints();
			ImGui::EndTabItem();
		}
		
        if (ImGui::BeginTabItem("Watches"))
		{
			DrawWatches();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Registers"))
		{
            DrawRegisters(*pCodeAnalysis);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Stack"))
		{
			DrawStack();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Call Stack"))
		{
			DrawCallStack();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Trace"))
		{
			DrawTrace();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Events"))
		{
			DrawEvents();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
#endif
}

void FDebugger::FixupAddresRefs(void)
{
	for (auto& watch : Watches)
	{
		FixupAddressRef(*pCodeAnalysis, watch);
	}

	for (int i = 0; i < Breakpoints.size(); i++)
	{
		FixupAddressRef(*pCodeAnalysis, Breakpoints[i].Address);
	}

	FixupAddressRefList(*pCodeAnalysis, FrameTrace);
	FixupAddressRefList(*pCodeAnalysis, StackSetLocations);

	for (FCPUFunctionCall& functionCall : CallStack)
	{
		FixupAddressRef(*pCodeAnalysis, functionCall.CallAddr);
		FixupAddressRef(*pCodeAnalysis, functionCall.FunctionAddr);
		FixupAddressRef(*pCodeAnalysis, functionCall.ReturnAddr);
	}

	for (FEvent& event : EventTrace)
	{
		FixupAddressRef(*pCodeAnalysis, event.PC);
	}
}