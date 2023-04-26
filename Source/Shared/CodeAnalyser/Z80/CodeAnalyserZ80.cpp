#include "CodeAnalyserZ80.h"
#include "../CodeAnalyser.h"
#include <cassert>

#include "chips/z80.h"


bool CheckPointerIndirectionInstructionZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = state.ReadByte(pc);

	switch (instrByte)
	{
		// LD (nnnn),x
	case 0x22:
	case 0x32:
		// LD x,(nnnn)
	case 0x2A:
	case 0x3A:
		*out_addr = state.ReadWord(pc + 1);
		return true;
	
		// extended instructions
	case 0xED:
	{
		const uint8_t exInstrByte = state.ReadByte(pc + 1);
		switch (exInstrByte)
		{
		case 0x43://ld (**),bc
		case 0x4B://ld bc,(**)
		case 0x53://ld (**),de
		case 0x5B://ld de,(**)
		case 0x63://ld (**),hl
		case 0x6B://ld hl,(**)
		case 0x73://ld (**),sp
		case 0x7B://ld sp,(**)
			*out_addr = state.ReadWord(pc + 2);
			return true;
		}

	}
	return false;

	// IX/IY instructions
	case 0xDD:
	case 0xFD:
	{
		const uint8_t exInstrByte = state.ReadByte(pc + 1);
		switch (exInstrByte)
		{
		case 0x22://ld (**),ix/iy
		case 0x2A://ld ix/iy,(**)
			*out_addr = state.ReadWord(pc + 2);
			return true;
		}
	}
	}

	return false;
}


bool CheckPointerRefInstructionZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	if (CheckPointerIndirectionInstructionZ80(state, pc, out_addr))
		return true;

	const uint8_t instrByte = state.ReadByte(pc);

	switch (instrByte)
	{
		// LD x,nnnn
	case 0x01:
	case 0x11:
	case 0x21:
	case 0x31:
		*out_addr = state.ReadWord(pc + 1);
		return true;

		// IX/IY instructions
	case 0xDD:
	case 0xFD:
		{
			const uint8_t exInstrByte = state.ReadByte(pc + 1);
			switch (exInstrByte)
			{
			case 0x21://ld ix/iy,**
				*out_addr = state.ReadWord(pc + 2);
				return true;
			}
		}
	}
	return false;
}

bool CheckJumpInstructionZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = state.ReadByte(pc);

	switch (instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* JP nnnn */
	case 0xC3:
		/* JP cc,nnnn */
	case 0xDA: case 0xFA: case 0xD2: case 0xC2:
	case 0xF2: case 0xEA: case 0xE2: case 0xCA:
		*out_addr = state.ReadWord(pc + 1);
		return true;

		/* DJNZ d */
	case 0x10:
		/* JR d */
	case 0x18:
		/* JR cc,d */
	case 0x38: case 0x30: case 0x20: case 0x28:
	{
		const int8_t relJump = (int8_t)state.ReadByte(pc + 1);
		*out_addr = pc + 2 + relJump;	// +2 because it's relative to the next instruction
	}
	return true;
	/* RST */
	case 0xC7:  *out_addr = 0x00; return true;
	case 0xCF:  *out_addr = 0x08; return true;
	case 0xD7:  *out_addr = 0x10; return true;
	case 0xDF:  *out_addr = 0x18; return true;
	case 0xE7:  *out_addr = 0x20; return true;
	case 0xEF:  *out_addr = 0x28; return true;
	case 0xF7:  *out_addr = 0x30; return true;
	case 0xFF:  *out_addr = 0x38; return true;
	}

	return false;
}

bool CheckCallInstructionZ80(FCodeAnalysisState& state, uint16_t pc)
{
	const uint8_t instrByte = state.ReadByte(pc);

	switch (instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* RST */
	case 0xC7:
	case 0xCF:
	case 0xD7:
	case 0xDF:
	case 0xE7:
	case 0xEF:
	case 0xF7:
	case 0xFF:
		return true;
	}
	return false;
}

bool CheckStopInstructionZ80(FCodeAnalysisState& state, uint16_t pc)
{
	const uint8_t instrByte = state.ReadByte(pc);

	switch (instrByte)
	{
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:
		/* RST */
	case 0xC7:
	case 0xCF:
	case 0xD7:
	case 0xDF:
	case 0xE7:
	case 0xEF:
	case 0xF7:
	case 0xFF:
		// ret
	case 0xC8:
	case 0xC9:
	case 0xD8:
	case 0xE8:
	case 0xF8:


	case 0xc3:// jp
	case 0xe9:// jp(hl)
	case 0x18://jr
		return true;
	case 0xED:	// extended instructions
	{
		const uint8_t extInstrByte = state.ReadByte(pc + 1);
		switch (extInstrByte)
		{
		case 0x4D:	// more RET functions
		case 0x5D:
		case 0x6D:
		case 0x7D:
		case 0x45:
		case 0x55:
		case 0x65:
		case 0x75:
			return true;
		}
	}
	return false;
	// index register instructions
	case 0xDD:	// IX
	case 0xFD:	// IY
	{
		const uint8_t extInstrByte = state.ReadByte(pc + 1);
		switch (extInstrByte)
		{
		case 0xE9:	// JP(IX)
			return true;
		}
		return false;
	}
	default:
		return false;
	}
}

bool RegisterCodeExecutedZ80(FCodeAnalysisState& state, uint16_t pc, uint16_t oldpc)
{
	const ICPUInterface* pCPUInterface = state.CPUInterface;
	FDebugger& debugger = state.Debugger;
	const uint8_t opcode = pCPUInterface->ReadByte(pc);
	const uint8_t oldOpcode = pCPUInterface->ReadByte(oldpc);
	const z80_t* pCPU = static_cast<z80_t*>(state.CPUInterface->GetCPUEmulator());

	std::vector<FCPUFunctionCall>&	callStack = state.Debugger.GetCallstack();

	//const FZ80InternalState& cpuState = pCPU->internal_state;

	bool bPushInstruction = false;
	
	// check current op code
	switch (opcode)
	{
		// Stack
		case 0x31:	// LD SP,NN
			{
				const uint16_t newSP = state.ReadWord(pc + 1);
				debugger.RegisterNewStackPointer(newSP,state.AddressRefFromPhysicalAddress(pc));
			}
			break;
		case 0xF9:	// LD SP,HL
			debugger.RegisterNewStackPointer(pCPU->hl, state.AddressRefFromPhysicalAddress(pc));
			break;
		case 0xc5:	// PUSH BC
		case 0xd5:	// PUSH DE
		case 0xe5:	// PUSH HL
		case 0xf5:	// PUSH AF
			bPushInstruction = true;
		break;

		
		
		// Call functions
		/* CALL nnnn */
		case 0xCD:
			/* CALL cc,nnnn */
		case 0xDC: case 0xFC: case 0xD4: case 0xC4:
		case 0xF4: case 0xEC: case 0xE4: case 0xCC:
			bPushInstruction = true;
			break;

			// index register opcodes
		case 0xdd:
		case 0xfd:
		{
			const bool bIX = opcode == 0xDD;
			const uint8_t indexOpcode = pCPUInterface->ReadByte(pc + 1);
			switch (indexOpcode)
			{
			case 0xF9:	// LD SP,IX/IY
				debugger.RegisterNewStackPointer(bIX ? pCPU->ix : pCPU->iy, state.AddressRefFromPhysicalAddress(pc));
				break;
			case 0xe5:	// PUSH IX/IY
				bPushInstruction = true;
				break;
			default:
				break;
			}
		}
		break;

		case 0xed:
		{
			const uint8_t extendedOpcode = pCPUInterface->ReadByte(pc + 1);

			switch (extendedOpcode)
			{
				case 0x7b:	// LD SP,(nn)
				{
					const uint16_t newSP = state.ReadWord(state.ReadWord(pc + 2));	// indirect
					debugger.RegisterNewStackPointer(newSP, state.AddressRefFromPhysicalAddress(pc));
				}
				break;
			}

		}
			break;
	default:
		break;
	}

	// TODO: check old opcode for jump/ret instructions
	switch (oldOpcode)
	{
		// Call instructions
		/* CALL nnnn */
	case 0xCD:
		/* CALL cc,nnnn */
	case 0xDC: case 0xFC: case 0xD4: case 0xC4:
	case 0xF4: case 0xEC: case 0xE4: case 0xCC:

		if (pc != oldpc + 3)	// call instructions are 3 bytes
		{
			FCPUFunctionCall callInfo;
			callInfo.CallAddr = state.AddressRefFromPhysicalAddress(oldpc);
			callInfo.FunctionAddr = state.AddressRefFromPhysicalAddress(pc);
			callInfo.ReturnAddr = state.AddressRefFromPhysicalAddress(oldpc + 3);
			callStack.push_back(callInfo);
		}

		break;
		// ret
	case 0xC0:
	case 0xC8:
	case 0xC9:
	case 0xD0:
	case 0xD8:
	case 0xE0:
	case 0xE8:
	case 0xF0:
	case 0xF8:
		if (pc != oldpc + 1)	// ret instructions are 1 byte so if we're not on the next instruction, we've returned
		{
			if (callStack.empty() == false)
			{
				FCPUFunctionCall& callInfo = callStack.back();
				//assert(callInfo.ReturnAddr == nextpc);

				callStack.pop_back();

				/*if (callInfo.ReturnAddr != nextpc)
				{
					fprintf(stderr, "PC: $%04x, NextPC $%04x, Return Address $%04x\n", pc, nextpc, callInfo.ReturnAddr);
					return true;
				}*/

			}
		}
		break;
	}

	// Handle push instruction
	// store the comment from the code line that did the push at the location in the stack as a comment
	if(bPushInstruction)
	{
		const uint16_t stackPointer = pCPU->sp - 2;

		if (state.Debugger.IsAddressOnStack(stackPointer))
		{
			FDataInfo* pStackItem = state.GetWriteDataInfoForAddress(stackPointer);	// -2 because SP was recorded before instruction was 	
			const FCodeInfo* pCodeItem = state.GetCodeInfoForAddress(pc);

			if (pCodeItem != nullptr)// && pCodeItem->Comment.empty() == false)
				pStackItem->Comment = pCodeItem->Comment;
			else
				pStackItem->Comment = "";

			// Format stack data item
			if (pStackItem->DataType != EDataType::Word)
			{
				pStackItem->DataType = EDataType::Word;
				pStackItem->ByteSize = 2;
				state.SetCodeAnalysisDirty(stackPointer);
			}
		}
	}

	return false;
}

std::vector<FMachineStateZ80*> g_FreeMachineStates;
std::vector<FMachineStateZ80*> g_AllocatedMachineStates;

// Machine state & capture
FMachineStateZ80* AllocateMachineStateZ80()
{
	FMachineStateZ80* pNewState = nullptr;
	
	if (g_FreeMachineStates.empty())
	{
		pNewState = new FMachineStateZ80;
	}
	else
	{
		pNewState = g_FreeMachineStates.back();
		g_FreeMachineStates.pop_back();
	}

	g_AllocatedMachineStates.push_back(pNewState);
	return pNewState;
}

void FreeMachineStatesZ80()
{
	for (FMachineStateZ80* pState : g_AllocatedMachineStates)
	{
		g_FreeMachineStates.push_back(pState);
	}
	g_AllocatedMachineStates.clear();
}

void CaptureMachineStateZ80(FMachineState* pMachineState, ICPUInterface* pCPUInterface)
{
	z80_t* pCPU = (z80_t*)pCPUInterface->GetCPUEmulator();
	FMachineStateZ80* pMachineStateZ80 = static_cast<FMachineStateZ80 *>(pMachineState);

	pMachineStateZ80->AF = pCPU->af;
	pMachineStateZ80->BC = pCPU->bc;
	pMachineStateZ80->DE = pCPU->de;
	pMachineStateZ80->HL = pCPU->hl;
	pMachineStateZ80->AF_ = pCPU->af2;
	pMachineStateZ80->BC_ = pCPU->bc2;
	pMachineStateZ80->DE_ = pCPU->de2;
	pMachineStateZ80->HL_ = pCPU->hl2;
	pMachineStateZ80->IX = pCPU->ix;
	pMachineStateZ80->IY = pCPU->iy;
	pMachineStateZ80->SP = pCPU->sp;
	pMachineStateZ80->PC = pCPU->pc;
	pMachineStateZ80->I = pCPU->i;
	pMachineStateZ80->R = pCPU->r;
	pMachineStateZ80->IM = pCPU->im;

	for (int stackVal = 0; stackVal < FMachineStateZ80::kNoStackEntries; stackVal++)
		pMachineStateZ80->Stack[stackVal] = pCPUInterface->ReadWord(pMachineStateZ80->SP - (stackVal * 2));
}