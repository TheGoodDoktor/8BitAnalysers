#include "Debugger.h"
#include "SpectrumEmu.h"

void FDebugger::Init(FSpectrumEmu* pEmu)
{
	pEmulator = pEmu;
}

void FDebugger::CPUTick(uint64_t pins)
{
    uint64_t risingPins = pins & (pins ^ LastTickPins);
    int trapId = kTrapId_None;

    const uint16_t addr = Z80_GET_ADDR(pins);
    const bool bMemAccess = !!((pins & Z80_CTRL_PIN_MASK) & Z80_MREQ);
    const bool bWrite = (pins & Z80_CTRL_PIN_MASK) == (Z80_MREQ | Z80_WR);

    const z80_t& cpu = pEmulator->ZXEmuState.cpu;
    const bool bNewOp = z80_opdone(&pEmulator->ZXEmuState.cpu);

    if (bNewOp)
    {
        PC = pEmulator->CodeAnalysis.AddressRefFromPhysicalAddress(pins & 0xffff);
        //FAddressRef pc = pEmulator->CodeAnalysis.AddressRefFromPhysicalAddress(cpu.pc);

        if (StepMode != EDebugStepMode::None)
        {
            switch (StepMode)
            {
            case EDebugStepMode::StepInto:
                trapId = kTrapId_Step;
                break;
            case EDebugStepMode::StepOver:
                // TODO: check against step over PC value and stop
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
                    case EBreakpointType::Data:
                        break;
                    }
                }
            }
        }
    }

    // tick based stepping
    switch (StepMode)
    {
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

    if (trapId != kTrapId_None)
    {
        Break();
    }

    LastTickPins = pins;
#if 0
    for (int i = 0; (i < win->dbg.num_breakpoints) && (trap_id == 0); i++) {
        const ui_dbg_breakpoint_t* bp = &win->dbg.breakpoints[i];
        if (bp->enabled) {
            switch (bp->type) {
            case UI_DBG_BREAKTYPE_IRQ:
#if defined(UI_DBG_USE_Z80)
                if (Z80_INT & rising_pins) {
                    trap_id = UI_DBG_BP_BASE_TRAPID + i;
                }
#elif defined(UI_DBG_USE_M6502)
                if (M6502_IRQ & rising_pins) {
                    trap_id = UI_DBG_BP_BASE_TRAPID + i;
                }
#endif
                break;

            case UI_DBG_BREAKTYPE_NMI:
#if defined(UI_DBG_USE_Z80)
                if (Z80_NMI & rising_pins) {
                    trap_id = UI_DBG_BP_BASE_TRAPID + i;
                }
#elif defined(UI_DBG_USE_M6502)
                if (M6502_NMI & rising_pins) {
                    trap_id = UI_DBG_BP_BASE_TRAPID + i;
                }
#endif
                break;

#if defined(UI_DBG_USE_Z80)
            case UI_DBG_BREAKTYPE_OUT:
                if ((pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_WR)) {
                    const uint16_t mask = bp->val;
                    if ((Z80_GET_ADDR(pins) & mask) == (bp->addr & mask)) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                }
                break;

            case UI_DBG_BREAKTYPE_IN:
                if ((pins & Z80_CTRL_PIN_MASK) == (Z80_IORQ | Z80_RD)) {
                    const uint16_t mask = bp->val;
                    if ((Z80_GET_ADDR(pins) & mask) == (bp->addr & mask)) {
                        trap_id = UI_DBG_BP_BASE_TRAPID + i;
                    }
                }
                break;
#endif
            }
        }
    }
#endif
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

void	FDebugger::StepOver()
{
    // TODO: this one's a bit more tricky!
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


bool FDebugger::IsAddressBreakpointed(FAddressRef addr)
{
	for (int i = 0; i < Breakpoints.size(); i++)
	{
		if (Breakpoints[i].Address == addr)
			return true;
	}
	return false;
}
