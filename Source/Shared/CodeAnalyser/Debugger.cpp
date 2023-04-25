#include "Debugger.h"

#include <CodeAnalyser/CodeAnalyser.h>

#include <chips/z80.h>
#include <chips/util/z80dasm.h>
#include <chips/util/m6502dasm.h>

#include <imgui.h>
#include "UI/CodeAnalyserUI.h"

void FDebugger::Init(FCodeAnalysisState* pCA)
{
	pCodeAnalysis = pCA;
    CPUType = pCodeAnalysis->GetCPUInterface()->CPUType;

    if(CPUType == ECPUType::Z80)
        pZ80 = (z80_t*)pCodeAnalysis->GetCPUInterface()->GetCPUEmulator();
    if (CPUType == ECPUType::M6502)
        pM6502 = (m6502_t*)pCodeAnalysis->GetCPUInterface()->GetCPUEmulator();

    Watches.clear();
}

void FDebugger::CPUTick(uint64_t pins)
{
    const uint64_t risingPins = pins & (pins ^ LastTickPins);
    int trapId = kTrapId_None;

    uint16_t addr = 0;
	bool bMemAccess = false;
	bool bWrite = false;
	bool bNewOp = false;

    if (CPUType == ECPUType::Z80)
    {
        addr = Z80_GET_ADDR(pins);
        bMemAccess = !!((pins & Z80_CTRL_PIN_MASK) & Z80_MREQ);
        bWrite = (pins & Z80_CTRL_PIN_MASK) == (Z80_MREQ | Z80_WR);
        bNewOp = z80_opdone(pZ80);
    }
    else if (CPUType == ECPUType::M6502)
    {
        // TODO: 6502 version
		addr = M6502_GET_ADDR(pins);
		bNewOp =  pins & M6502_SYNC;
	}

    const FAddressRef addrRef = pCodeAnalysis->AddressRefFromPhysicalAddress(addr);
    //const z80_t& cpu = pEmulator->ZXEmuState.cpu;

    if (bNewOp)
    {
        PC = pCodeAnalysis->AddressRefFromPhysicalAddress(pins & 0xffff);

        if (StepMode != EDebugStepMode::None)
        {
            switch (StepMode)
            {
            case EDebugStepMode::StepInto:
                trapId = kTrapId_Step;
                break;
            case EDebugStepMode::StepOver:
                // Check against step over PC value and stop
                if(PC == StepOverPC)
					trapId = kTrapId_Step;
				break;

            }
        }
        else
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

    }

    // tick based stepping
    switch (StepMode)
    {
        // This is ZX Spectrum specific - need to think of a generic way of doing it - large memory breakpoint?
        case EDebugStepMode::ScreenWrite:
        {
            // break on screen memory write
            if (bWrite && addr >= 0x4000 && addr < 0x5800)
            {
                trapId = kTrapId_Step;
            }
        }
        break;
    }

    // iterate through data breakpoints
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
                if (CPUType == ECPUType::Z80 && risingPins & Z80_INT)
					trapId = kTrapId_BpBase + i;
				else if (CPUType == ECPUType::M6502 && risingPins & M6502_IRQ)
					trapId = kTrapId_BpBase + i;
				break;

			case EBreakpointType::NMI:
				if (CPUType == ECPUType::Z80 && risingPins & Z80_NMI)
					trapId = kTrapId_BpBase + i;
				else if (CPUType == ECPUType::M6502 && risingPins & M6502_NMI)
					trapId = kTrapId_BpBase + i;
				break;

            // In/Out - only for Z80
			case EBreakpointType::In:
				if (CPUType == ECPUType::Z80 && (pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_RD))
                {
					const uint16_t mask = bp.Val;
					if ((Z80_GET_ADDR(pins) & mask) == (bp.Address.Address & mask))
						trapId = kTrapId_BpBase + i;
				}
				break;

			case EBreakpointType::Out:
				if (CPUType == ECPUType::Z80 && (pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_WR))
                {
					const uint16_t mask = bp.Val;
					if ((Z80_GET_ADDR(pins) & mask) == (bp.Address.Address & mask))
						trapId = kTrapId_BpBase + i;
				}
				break;
			}
		}
	}

    if (trapId != kTrapId_None)
    {
        Break();
    }

    LastTickPins = pins;
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

// TODO: Load state - breakpoints, watches etc.
void	FDebugger::LoadFromFile(FILE* fp)
{

}

// TODO: Save state - breakpoints, watches etc.
void	FDebugger::SaveToFile(FILE* fp)
{

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

struct FStepDasmData
{
    std::vector<uint8_t> Data;
    FCodeAnalysisState* pCodeAnalysis = nullptr;
    uint16_t    PC;
};

static uint8_t StepOverDasmInCB(void* userData)
{
    FStepDasmData* pDasmData = (FStepDasmData*)userData;

    // Get Opcode bytes
    uint8_t opcodeByte = pDasmData->pCodeAnalysis->ReadByte(pDasmData->PC++);
    pDasmData->Data.push_back(opcodeByte);
    return opcodeByte;
}

static void StepOverDasmOutCB(char c, void* userData)
{
    FStepDasmData* pDasmData = (FStepDasmData*)userData;
    // do we need to do anything here?
}

void	FDebugger::StepOver()
{
	//const ECPUType cpuType = pEmulator->CPUType;

	// TODO: this one's a bit more tricky!
    FStepDasmData dasmData;
    dasmData.PC = PC.Address;
    dasmData.pCodeAnalysis = pCodeAnalysis;
    bDebuggerStopped = false;
    uint16_t nextPC = 0;
	if (CPUType == ECPUType::Z80)
        nextPC = z80dasm_op(PC.Address, StepOverDasmInCB, StepOverDasmOutCB, &dasmData);
    else
        nextPC = m6502dasm_op(PC.Address, StepOverDasmInCB, StepOverDasmOutCB, &dasmData);

    if (IsStepOverOpcode(CPUType, dasmData.Data[0]))
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


bool FDebugger::IsAddressBreakpointed(FAddressRef addr) const
{
	for (int i = 0; i < Breakpoints.size(); i++)
	{
		if (Breakpoints[i].Address == addr)
			return true;
	}
	return false;
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

// UI Code

void FDebugger::DrawTrace(void)
{
	FCodeAnalysisState& state = *pCodeAnalysis;
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();
	const float line_height = ImGui::GetTextLineHeight();
	ImGuiListClipper clipper((int)FrameTrace.size(), line_height);

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
	const uint16_t sp = state.CPUInterface->GetSP();
	if (state.StackMin >= state.StackMax)	// stack is invalid
	{
		ImGui::Text("No valid stack discovered");
		return;
	}

	if (sp < state.StackMin || sp > state.StackMax)	// sp is not in range
	{
		ImGui::Text("Stack pointer: %s", NumStr(sp));
		DrawAddressLabel(state, state.GetFocussedViewState(), sp);
		ImGui::SameLine();
		ImGui::Text("not in stack range(%s - %s)", NumStr(state.StackMin), NumStr(state.StackMax));
		return;
	}

	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	//static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

	if (ImGui::BeginTable("stackinfo", 4, flags))
	{
		ImGui::TableSetupColumn("Address");
		ImGui::TableSetupColumn("Value");
		ImGui::TableSetupColumn("Comment");
		ImGui::TableSetupColumn("Set by");
		ImGui::TableHeadersRow();

		for (int stackAddr = sp; stackAddr <= state.StackMax; stackAddr += 2)
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
		FDataInfo* pDataInfo = state.GetReadDataInfoForAddress(watch.Address);
		ImGui::PushID(watch.Address);
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
				FDataInfo* pInfo = state.GetWriteDataInfoForAddress(SelectedWatch.Address);
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


void FDebugger::DrawUI(void)
{
    if (ImGui::BeginTabBar("DebuggerTabBar"))
    {
        if (ImGui::BeginTabItem("Breakpoints"))
        {

			ImGui::EndTabItem();
		}
		
        if (ImGui::BeginTabItem("Watches"))
		{

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

		ImGui::EndTabBar();
	}
}
