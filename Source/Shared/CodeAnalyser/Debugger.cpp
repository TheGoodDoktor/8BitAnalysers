#include "Debugger.h"

#include <CodeAnalyser/CodeAnalyser.h>

#include <chips/z80.h>

#include <imgui.h>
#include "UI/CodeAnalyserUI.h"
#include "Z80/Z80Disassembler.h"
#include "6502/M6502Disassembler.h"
#include <Util/GraphicsView.h>

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

    if(CPUType == ECPUType::Z80)
        pZ80 = (z80_t*)pCodeAnalysis->GetCPUInterface()->GetCPUEmulator();
    if (CPUType == ECPUType::M6502)
        pM6502 = (m6502_t*)pCodeAnalysis->GetCPUInterface()->GetCPUEmulator();

    Watches.clear();
	Stacks.clear();

	StackMin = 0xffff;
	StackMax = 0;
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
	uint32_t BPMaskCheck = 0;

    if (CPUType == ECPUType::Z80)
    {
        addr = Z80_GET_ADDR(pins);

        bMemAccess = !!((pins & Z80_CTRL_PIN_MASK) & Z80_MREQ);
		bWrite = (risingPins & Z80_CTRL_PIN_MASK) == (Z80_MREQ | Z80_WR);
		bRead = (risingPins & Z80_CTRL_PIN_MASK) == (Z80_MREQ | Z80_RD);
        bNewOp = z80_opdone(pZ80);
		bIORead = (pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_RD);
		bIOWrite = (pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_WR);
		bIrq = (pins & Z80_INT) && pZ80->iff1;
		bNMI = risingPins & Z80_NMI;
    }
    else if (CPUType == ECPUType::M6502)
    {
		addr = M6502_GET_ADDR(pins);
		// TODO: bMemAccess
		// TODO: bWrite
		// TODO: bRead
		bNewOp =  pins & M6502_SYNC;
		bIrq = risingPins & M6502_IRQ;
		bNMI = risingPins & M6502_NMI;
	}

	// setup breakpoint mask to check
	BPMaskCheck |= bWrite ? BPMask_DataWrite : 0;

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
            if (bWrite && addr >= ScreenMemoryStart && addr <= ScreenMemoryEnd)
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
    }

    // iterate through data breakpoints
	// this can slow down if there are a lot of BPs
	// Do a mask check
	if (BPMaskCheck & BreakpointMask)
	{
		for (int i = 0; i < Breakpoints.size(); i++)
		{
			const FBreakpoint& bp = Breakpoints[i];

			if (bp.bEnabled)
			{
				switch (bp.Type)
				{
				case EBreakpointType::Data:
					if (bWrite &&
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
	int trapId = kTrapId_None;

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

		}
	}
	else if(BreakpointMask & BPMask_Exec)
	{
		for (int i = 0; i < Breakpoints.size(); i++)
		{
			const FBreakpoint& bp = Breakpoints[i];

			if (bp.bEnabled)
			{
				switch (bp.Type)
				{
				case EBreakpointType::Exec:
					if (PC == bp.Address)
					{
						trapId = kTrapId_BpBase + i;
					}
					break;
				}
			}
		}
	}

	// Handle IRQ
	//  Note: This is Z80 specific
	const bool irq = (pins & Z80_INT) && pZ80->iff1;
	if (irq)
	{
		FCPUFunctionCall callInfo;
		callInfo.CallAddr = PC;
		callInfo.FunctionAddr = PC;
		callInfo.ReturnAddr = PC;
		CallStack.push_back(callInfo);
		//return UI_DBG_BP_BASE_TRAPID + 255;	//hack
	}

	FrameTrace.push_back(PC);

	// update stack size
	const uint16_t sp = pZ80->sp;	// this won't get the proper stack pos (see comment above function)
	if (sp == StackMin - 2 || StackMin == 0xffff)
		StackMin = sp;
	if (sp == StackMax + 2 || StackMax == 0)
		StackMax = sp;

	return trapId;
}

void FDebugger::StartFrame() 
{ 
	if (bClearEventsEveryFrame)
		ClearEvents();
	FrameTrace.clear();

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
			}
		}
	}
}

bool FDebugger::FrameTick(void)
{
	// handle frame stepping
	if (StepMode == EDebugStepMode::Frame)
	{
		StepMode = EDebugStepMode::None;
		Break();
	}

	return bDebuggerStopped;
}

static const uint32_t kVersionNo = 2;

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
	}

	// frame trace
	if (versionNo > 1)
	{
		FrameTrace.clear();
		fread(&num, sizeof(uint32_t), 1, fp);
		for (int i = 0; i < (int)num; i++)
		{
			FAddressRef& address = FrameTrace.emplace_back();
			fread(&address.Val, sizeof(uint32_t), 1, fp);	// address
		}
	}
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
}


void FDebugger::Break()
{ 
    StepMode = EDebugStepMode::None;
    bDebuggerStopped = true;
}

void FDebugger::Continue()
{ 
    StepMode = EDebugStepMode::None; 
    bDebuggerStopped = false; 
}


void FDebugger::StepInto()
{
    StepMode = EDebugStepMode::StepInto;
    bDebuggerStopped = false;
}

// check if the an instruction is a 'step over' op 
static bool IsStepOverOpcode(ECPUType cpuType, uint8_t opcode)
{
    if (cpuType == ECPUType::Z80)
    {
        switch (opcode)
        {
            // CALL nnnn 
        case 0xCD:
            // CALL cc,nnnn 
        case 0xDC: case 0xFC: case 0xD4: case 0xC4:
        case 0xF4: case 0xEC: case 0xE4: case 0xCC:
            // DJNZ d 
        case 0x10:
            return true;
        default:
            return false;
        }
    }
    else if (cpuType == ECPUType::M6502)
    {
        // on 6502, only JSR qualifies 
        return opcode == 0x20;
    }
    else
    {
        return false;
    }
}

void	FDebugger::StepOver()
{
	//const ECPUType cpuType = pEmulator->CPUType;

	uint8_t stepOpcode = 0;
	// TODO: this one's a bit more tricky!
   
    bDebuggerStopped = false;
    uint16_t nextPC = 0;
	if (CPUType == ECPUType::Z80)
		nextPC = Z80DisassembleGetNextPC(PC.Address, *pCodeAnalysis, stepOpcode);
    else if (CPUType == ECPUType::M6502)
		nextPC = M6502DisassembleGetNextPC(PC.Address, *pCodeAnalysis, stepOpcode);

    if (IsStepOverOpcode(CPUType, stepOpcode))
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

	Breakpoints.emplace_back(addr, EBreakpointType::Exec);
	return true;
}

bool FDebugger::AddDataBreakpoint(FAddressRef addr, uint16_t size)
{
	if (IsAddressBreakpointed(addr))
		return false;

	Breakpoints.emplace_back(addr, EBreakpointType::Data,size);
	return true;
}

bool FDebugger::RemoveBreakpoint(FAddressRef addr)
{
	for (int i = 0; i < Breakpoints.size(); i++)
	{
		if (Breakpoints[i].Address == addr)
		{
			Breakpoints[i] = Breakpoints.back();
			Breakpoints.pop_back();
			return true;
		}
	}
	return false;
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
	for (auto watchIt = Watches.begin(); watchIt != Watches.end(); ++watchIt)
	{
		if (*watchIt == watch)
			Watches.erase(watchIt);
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

FEventTypeInfo g_EventTypeInfo[256];

void FDebugger::RegisterEventType(uint8_t type, const char* pName, uint32_t col, ShowEventInfoCB pShowAddress, ShowEventInfoCB pShowValue)
{
	FEventTypeInfo& typeInfo = g_EventTypeInfo[type];
	assert(strlen(pName) < kEventNameLength);
	strncpy(typeInfo.EventName, pName, kEventNameLength);
	typeInfo.EventColour = col;
	typeInfo.ShowAddressCB = pShowAddress;
	typeInfo.ShowValueCB = pShowValue;
}

void FDebugger::RegisterEvent(uint8_t type, FAddressRef pc, uint16_t address, uint8_t value, uint16_t scanlinePos)
{
	if (!g_EventTypeInfo[type].bEnabled)
		return;

	ScanlineEvents[scanlinePos] = type;
	EventTrace.emplace_back(type, pc, address, value, scanlinePos);
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
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)FrameTrace.size(), line_height);

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
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const FAddressRef codeAddress = FrameTrace[FrameTrace.size() - i - 1];
				FCodeInfo* pCodeInfo = state.GetCodeInfoForAddress(codeAddress);
				DrawCodeAddress(state, viewState, codeAddress, false);	// draw current PC
				//DrawCodeInfo(state, viewState, FCodeAnalysisItem(pCodeInfo, codeAddress));
			}
		}
	}
	ImGui::EndChild();
}

void FDebugger::DrawCallStack(void)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	// Draw current function & PC position
	if (CallStack.empty() == false)
	{
		const FLabelInfo* pLabel = state.GetLabelForAddress(CallStack.back().FunctionAddr);
		if (pLabel != nullptr)
		{
			ImGui::Text("%s :", pLabel->Name.c_str());
			ImGui::SameLine();
		}
	}
	DrawCodeAddress(state, viewState, state.CPUInterface->GetPC(), false);	// draw current PC

	for (int i = (int)CallStack.size() - 1; i >= 0; i--)
	{
		if (i > 0)
		{
			const FLabelInfo* pLabel = state.GetLabelForAddress(CallStack[i - 1].FunctionAddr);
			if (pLabel != nullptr)
			{
				ImGui::Text("%s :", pLabel->Name.c_str());
				ImGui::SameLine();
			}
		}
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

void DrawRegisters(FCodeAnalysisState& state)
{
	if (state.CPUInterface->CPUType == ECPUType::Z80)
		DrawRegisters_Z80(state);
}

void FDebugger::DrawWatches(void)
{
    FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	bool bDeleteSelectedWatch = false;

	for (const auto& watch : Watches)
	{
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(watch);
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
				FDataInfo* pInfo = state.GetWriteDataInfoForAddress(SelectedWatch);
				state.ToggleDataBreakpointAtAddress(SelectedWatch, pInfo->ByteSize);
			}

			ImGui::EndPopup();
		}
		ImGui::SetItemAllowOverlap();	// allow buttons
		ImGui::SameLine();
		DrawDataInfo(state, viewState, FCodeAnalysisItem(pDataInfo, watch.BankId, watch.Address), true, true);

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
	}

	return "Unknown";
}

void FDebugger::DrawBreakpoints(void)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable("Breakpoints", 4, flags))
	{
		ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 60);
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed,50);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed,40);
		ImGui::TableHeadersRow();

		for (auto& bp : Breakpoints)
		{
			ImGui::PushID(bp.Address.Val);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Checkbox("##Enabled", &bp.bEnabled);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s:", NumStr(bp.Address.Address));
			DrawAddressLabel(state, viewState, bp.Address);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%s", GetBreakpointTypeText(bp.Type));
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%d", bp.Size);
			ImGui::PopID();
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
	const uint8_t pixels = event.Value;
	//dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize * 8, pos.y + rectSize), 0xffffffff);

	for (int i = 0; i < 8; i++)
	{
		if (pixels & (1 << (7 - i)))
			dl->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);

		dl->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + rectSize, pos.y + rectSize), 0xffffffff);
		pos.x += rectSize;
	}

	// tool tip?
	//ImGui::Text("%s", NumStr(event.Value));
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
	if (ImGui::Button("Clear"))
	{
		ClearEvents();
	}
	ImGui::Checkbox("Clear Every Frame", &bClearEventsEveryFrame);

	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const float lineHeight = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)EventTrace.size(), lineHeight);
	const float rectSize = lineHeight;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	
	if (ImGui::CollapsingHeader("Event Types"))
	{
		int e = 1;	// skip event type None
		while (g_EventTypeInfo[e].EventName[0])
		{
			ImVec2 pos = ImGui::GetCursorScreenPos();
			const ImVec2 rectMin(pos.x, pos.y + 3);
			const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize + 3);
			dl->AddRectFilled(rectMin, rectMax, g_EventTypeInfo[e].EventColour);

			ImGui::Text("  "); 
			ImGui::SameLine();
			ImGui::Checkbox(g_EventTypeInfo[e].EventName, &g_EventTypeInfo[e].bEnabled);
			e++;
		}
	}

	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY;
	if (ImGui::BeginTable("Events", 4, flags))
	{
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 150);
		ImGui::TableSetupColumn("PC", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 100);
		ImGui::TableHeadersRow();

		//if (ImGui::BeginChild("EventListChild"))
		{
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					const FEvent& event = EventTrace[i];
					const FEventTypeInfo& typeInfo = g_EventTypeInfo[event.Type];
					ImGui::PushID(i);
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImVec2 pos = ImGui::GetCursorScreenPos();
					
					// Type
					const ImVec2 rectMin(pos.x, pos.y);
					const ImVec2 rectMax(pos.x + rectSize, pos.y + rectSize);
					dl->AddRectFilled(rectMin, rectMax, typeInfo.EventColour);

					ImGui::Text("   %s", GetEventName(event.Type));
					
					// PC
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%s:", NumStr(event.PC.Address));
					DrawAddressLabel(state, viewState, event.PC);

					// Address
					ImGui::TableSetColumnIndex(2);
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
					ImGui::TableSetColumnIndex(3);
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
		}
		//ImGui::EndChild();

		ImGui::EndTable();

	}
}

void FDebugger::DrawUI(void)
{
	if (ImGui::Button("Step IO Read"))
		StepIORead();
	ImGui::SameLine();
	if (ImGui::Button("Step IO Write"))
		StepIOWrite();

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
}
