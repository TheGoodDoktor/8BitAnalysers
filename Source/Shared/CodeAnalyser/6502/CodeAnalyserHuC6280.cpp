#include "CodeAnalyserHuC6280.h"
#include "../CodeAnalyser.h"
#include <Debug/DebugLog.h>

enum class EAddressMode : uint8_t
{
	ZPIndirect_X,
	ZP,
	Immediate,
	Absolute,
	ZPIndirect_Y,
	ZP_X,
	Absolute_Y,
	Absolute_X,
	Accumulator,
	NA
};

static EAddressMode g_Group00_AddressModes[8] =
{
	EAddressMode::Immediate,	// 000
	EAddressMode::ZP,			// 001
	EAddressMode::NA,			// 010 - missing
	EAddressMode::Absolute,		// 011
	EAddressMode::NA,			// 100 - missing
	EAddressMode::ZP_X,			// 101
	EAddressMode::NA,			// 110 - missing
	EAddressMode::Absolute_X,	// 111
};

static EAddressMode g_Group01_AddressModes[8] =
{
	EAddressMode::ZPIndirect_X,
	EAddressMode::ZP,
	EAddressMode::Immediate,
	EAddressMode::Absolute,
	EAddressMode::ZPIndirect_Y,
	EAddressMode::ZP_X,
	EAddressMode::Absolute_Y,
	EAddressMode::Absolute_X,
};

static EAddressMode g_Group10_AddressModes[8] =
{
	EAddressMode::Immediate,
	EAddressMode::ZP,
	EAddressMode::Accumulator,
	EAddressMode::Absolute,
	EAddressMode::NA,	// 100 - missing
	EAddressMode::ZP_X,
	EAddressMode::NA,	// 110 - missing
	EAddressMode::Absolute_X,
};

EAddressMode GetInstructionAddressModeHuC6280(uint8_t opcode)
{
	const uint8_t instrGroup = opcode & 3;
	const uint8_t addrMode = (opcode >> 2) & 7;

	switch (instrGroup)
	{
		case 0x00:
			return g_Group00_AddressModes[addrMode];
		case 0x01:
			return g_Group01_AddressModes[addrMode];
		case 0x02:
			return g_Group10_AddressModes[addrMode];
	}

	return EAddressMode::NA;
}

bool CheckPointerIndirectionInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = state.ReadByte(pc);

	// use switch to catch specifics
	/*switch (instrByte)
	{
	case 0x6C:	// JMP (addr)
		*out_addr = pCPUInterface->ReadWord(pc + 1);
		return true;
	}*/

	// Implied HuC6280-specific instructions that the address mode table misidentifies
	switch (instrByte)
	{
	case 0x54:	// CSL - implied
	case 0xD4:	// CSH - implied
	case 0xF4:	// SET - implied
		return false;
	}

	// otherwise decode addressing mode
	const EAddressMode addrMode = GetInstructionAddressModeHuC6280(instrByte);

	switch (addrMode)
	{
	case EAddressMode::ZPIndirect_X:
	case EAddressMode::ZPIndirect_Y:
		*out_addr = state.ReadByte(pc + 1);
		return true;

	case EAddressMode::Absolute:
	case EAddressMode::Absolute_X:
	case EAddressMode::Absolute_Y:
		*out_addr = state.ReadWord(pc + 1);
		return true;

	case EAddressMode::ZP:
	case EAddressMode::ZP_X:
		*out_addr = state.ReadByte(pc + 1);
		return true;
    default:
        return false;
	}
/*
	switch (instrByte)
	{
	case 0x61:	// ADC (zp addr,X)
	case 0x71:	// ADC (zp addr),Y

	case 0xa1:	// LDA (zp addr,X)
	case 0xb1:	// LDA (zp addr),Y
	case 0x81:	// STA (zp addr,X)
	case 0x91:	// STA (zp addr),Y
		*out_addr = pCPUInterface->ReadByte(pc + 1);
		return true;
	}*/
	return false;
}



bool CheckPointerRefInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = state.ReadByte(pc);

	//if (CheckPointerIndirectionInstruction6502(state, pc, out_addr))
	//	return true;

	// use switch to catch specifics
	/*switch (instrByte)
	{
	}*/

	// otherwise decode addressing mode
#if 0
	const EAddressMode addrMode = GetInstructionAddressMode(instrByte);

	switch (addrMode)
	{
	case EAddressMode::Absolute:
	case EAddressMode::Absolute_X:
	case EAddressMode::Absolute_Y:
		*out_addr = state.ReadWord(pc + 1);
		return true;

	case EAddressMode::ZP:
	case EAddressMode::ZP_X:
		*out_addr = state.ReadByte(pc + 1);
		return true;
	}
#endif
	/*switch (instrByte)
	{
		// full address
	case 0x6d:	// ADC <addr>
	case 0x7d:	// ADC <addr>,X
	case 0x79:	// ADC <addr>,Y
	case 0x2d:	// AND <addr>
	case 0x3d:	// AND <addr>,X
	case 0x39:	// AND <addr>,Y

	case 0xad:	// LDA <addr>
	case 0xbd:	// LDA <addr>,X
	case 0xb9:	// LDA <addr>,Y
	case 0xae:	// LDX <addr>
	case 0xbe:	// LDX <addr>,Y
	case 0xac:	// LDY <addr>
	case 0xbc:	// LDY <addr>,X

	case 0x8d:	// STA <addr>
	case 0x9d:	// STA <addr>,X
	case 0x99:	// STA <addr>,Y
	case 0x8e:	// STX <addr>
	case 0x8c:	// STY <addr>
		*out_addr = pCPUInterface->ReadWord(pc + 1);
		return true;

		// zero page
	case 0x65:	// ADC <zp addr>
	case 0x75:	// ADC <zp addr>,X
	case 0x25:	// AND <zp addr>
	case 0x35:	// AND <zp addr>,X
	
	case 0xa5:	// LDA <zp addr>
	case 0xb5:	// LDA <zp addr>,X
	case 0xa6:	// LDX <zp addr>
	case 0xb6:	// LDX <zp addr>,Y
	case 0xa4:	// LDY <zp addr>
	case 0xb4:	// LDY <zp addr>,X

	case 0x85:	// STA <zp addr>
	case 0x95:	// STA <zp addr>,X
	case 0x86:	// STX <zp addr>
	case 0x96:	// STX <zp addr>,Y
	case 0x84:	// STY <zp addr>
	case 0x94:	// STY <zp addr>,X
		*out_addr = pCPUInterface->ReadByte(pc + 1);
		return true;
	}*/

	return false;
}

bool CheckJumpInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = state.ReadByte(pc);

	// BBR/BBS: all 16 opcodes have low nibble 0xF (cc=3, bbb∈{3,7})
	// Format: opcode, zp_addr, rel_offset — 3-byte instruction
	if ((instrByte & 0x0F) == 0x0F)
	{
		const int8_t relJump = (int8_t)state.ReadByte(pc + 2);
		*out_addr = pc + 3 + relJump;
		return true;
	}

	switch (instrByte)
	{
		// relative branches (2-byte: opcode, rel_offset)
		case 0x10:	// BPL
		case 0x30:	// BMI
		case 0x44:	// BSR HuC6280
		case 0x50:	// BVC
		case 0x70:	// BVS
		case 0x80:	// BRA
		case 0x90:	// BCC
		case 0xB0:	// BCS
		case 0xD0:	// BNE
		case 0xF0:	// BEQ
		{
			const int8_t relJump = (int8_t)state.ReadByte(pc + 1);
			*out_addr = pc + 2 + relJump;	// +2 because it's relative to the next instruction
			return true;
		}

		// absolute address (3-byte: opcode, addr_lo, addr_hi)
		case 0x20:	// JSR
		case 0x4C:	// JMP abs
		case 0x6C:	// JMP (ind)
			*out_addr = state.ReadWord(pc + 1);
			return true;
	}
	return false;
}

bool CheckCallInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc)
{
	const uint8_t instrByte = state.ReadByte(pc);

	switch (instrByte)
	{
	case 0x20:	// JSR
	case 0x44:	// BSR 6280
		return true;
	}
	return false;
}

bool CheckStopInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc)
{
	const uint8_t instrByte = state.ReadByte(pc);

	switch (instrByte)
	{
	case 0x00:	// BRK
	case 0x40:	// RTI
	case 0x60:	// RTS
	case 0x4C:	// JMP abs
	case 0x6C:	// JMP (ind)
	case 0x7C:	// JMP (abs,X) - computed jump, target not statically known
	case 0x80:	// BRA - unconditional relative branch
		return true;
	}
	return false;
}

bool RegisterCodeExecutedHuC6280(FCodeAnalysisState& state, uint16_t pc, uint16_t oldpc)
{
	const ICPUInterface* pCPUInterface = state.CPUInterface;
	const uint8_t opcode = pCPUInterface->ReadByte(pc);

	auto& callStack = state.Debugger.GetCallstack();

	switch (opcode)
	{
		case 0x20:  // JSR
		{
			FCPUFunctionCall callInfo;
			callInfo.CallAddr = state.GetCanonicalAddressRef(pc);
			callInfo.FunctionAddr = state.GetCanonicalAddressRef(state.ReadWord(pc + 1));
			callInfo.ReturnAddr = state.GetCanonicalAddressRef(pc + 3);
			callStack.push_back(callInfo);
		}
		break;

		case 0x44:  // BSR - branch to subroutine (relative, 2-byte instruction)
		{
			FCPUFunctionCall callInfo;
			callInfo.CallAddr = state.GetCanonicalAddressRef(pc);
			const int8_t relJump = (int8_t)pCPUInterface->ReadByte(pc + 1);
			callInfo.FunctionAddr = state.GetCanonicalAddressRef(pc + 2 + relJump);
			callInfo.ReturnAddr = state.GetCanonicalAddressRef(pc + 2);
			callStack.push_back(callInfo);
		}
		break;

		case 0x40:	// RTI
		case 0x60:	// RTS
			if (callStack.empty() == false)
				callStack.pop_back();
		break;
	}

	return false;
}

EInstructionType GetInstructionTypeHuC6280(FCodeAnalysisState& state, FAddressRef addr)
{
	const uint8_t instByte = state.ReadByte(addr);

	switch (instByte)
	{
		// Enable interrupts
		case 0x78:	// SEI
			return EInstructionType::EnableInterrupts;
		
		// Disable interrupts
		case 0x58:	// CLI
			return EInstructionType::DisableInterrupts;

		
		// JSR
		case 0x20:
		// BSR
		case 0x44:
			return EInstructionType::FunctionCall;

		default:
			return EInstructionType::Unknown;
	}
}

void FillCodeInfoOperandsHuC6280(FCodeAnalysisState& state, uint16_t pc, FCodeInfo* pCodeInfo)
{
	const FAddressRef pcAddrRef = state.GetCanonicalAddressRef(pc);
	const uint8_t instrByte = state.ReadByte(pc);

#if MULTIPLE_OPERANDS_SUPPORT
	// BBR/BBS: 3-byte (opcode, zp, rel). Operand 0 = ZP pointer, operand 1 = branch target.
	// Handled before standard jump path, which would otherwise overwrite operand 0.
	if ((instrByte & 0x0F) == 0x0F)
	{
		const uint8_t zpAddr = state.ReadByte(pc + 1);
		const int8_t relJump = (int8_t)state.ReadByte(pc + 2);
		const uint16_t branchTarget = pc + 3 + relJump;
		const FAddressRef zpAddrRef = state.GetCanonicalAddressRef(zpAddr);
		const FAddressRef branchAddrRef = state.GetCanonicalAddressRef(branchTarget);

		if (pCodeInfo->OperandType == EOperandType::Unknown)
			pCodeInfo->OperandType = EOperandType::Pointer;
		pCodeInfo->OperandAddress = zpAddrRef;

		pCodeInfo->ExtraOperands[0].Type = EOperandType::JumpAddress;
		pCodeInfo->ExtraOperands[0].Address = branchAddrRef;

		pCodeInfo->bIsCall = false;
		FLabelInfo* pBranchLabel = GenerateLabelForAddress(state, branchAddrRef, ELabelType::Code);
		if (pBranchLabel)
			pBranchLabel->References.RegisterAccess(pcAddrRef);
		return;
	}
#endif

	uint16_t jumpAddr;
	if (CheckJumpInstructionHuC6280(state, pc, &jumpAddr))
	{
		pCodeInfo->bIsCall = CheckCallInstructionHuC6280(state, pc);
		const FAddressRef jumpAddrRef = state.GetCanonicalAddressRef(jumpAddr);
		pCodeInfo->OperandAddress = jumpAddrRef;
		if (pCodeInfo->OperandType == EOperandType::Unknown)
			pCodeInfo->OperandType = EOperandType::JumpAddress;

		FLabelInfo* pLabel = GenerateLabelForAddress(state, jumpAddrRef, pCodeInfo->bIsCall ? ELabelType::Function : ELabelType::Code);
		if (pLabel)
			pLabel->References.RegisterAccess(pcAddrRef);
		return;
	}

	uint16_t ptr;
	if (CheckPointerRefInstructionHuC6280(state, pc, &ptr))
	{
		const FAddressRef ptrAddr = state.GetCanonicalAddressRef(ptr);
		pCodeInfo->OperandAddress = ptrAddr;
		if (pCodeInfo->OperandType == EOperandType::Unknown)
			pCodeInfo->OperandType = EOperandType::Pointer;
	}
	else if (CheckPointerIndirectionInstructionHuC6280(state, pc, &ptr))
	{
		const FAddressRef ptrAddr = state.GetCanonicalAddressRef(ptr);
		pCodeInfo->OperandAddress = ptrAddr;
		if (pCodeInfo->OperandType == EOperandType::Unknown)
			pCodeInfo->OperandType = EOperandType::Pointer;

		FLabelInfo* pLabel = GenerateLabelForAddress(state, ptrAddr, ELabelType::Data);
		if (pLabel)
			pLabel->References.RegisterAccess(pcAddrRef);
	}

#if MULTIPLE_OPERANDS_SUPPORT
	// HuC6280 multi-operand instructions
	switch (instrByte)
	{
		// Block transfers: opcode, src(16), dst(16), len(16)
		case 0x73:	// TII
		case 0xC3:	// TDD
		case 0xD3:	// TIN
		case 0xE3:	// TIA
		case 0xF3:	// TAI
		{
			const uint16_t src = state.ReadWord(pc + 1);
			const uint16_t dst = state.ReadWord(pc + 3);
			if (pCodeInfo->OperandType == EOperandType::Unknown)
				pCodeInfo->OperandType = EOperandType::Pointer;
			pCodeInfo->OperandAddress = state.GetCanonicalAddressRef(src);
			pCodeInfo->ExtraOperands[0].Type = EOperandType::Pointer;
			pCodeInfo->ExtraOperands[0].Address = state.GetCanonicalAddressRef(dst);
			pCodeInfo->ExtraOperands[1].Type = EOperandType::Hex;
			break;
		}

		// TST imm,zp / imm,zp,X
		case 0x83:
		case 0xA3:
		{
			const uint16_t zpAddr = state.ReadByte(pc + 2);
			pCodeInfo->OperandType = EOperandType::Hex;
			pCodeInfo->OperandAddress = FAddressRef::Invalid();
			pCodeInfo->ExtraOperands[0].Type = EOperandType::Pointer;
			pCodeInfo->ExtraOperands[0].Address = state.GetCanonicalAddressRef(zpAddr);
			break;
		}

		// TST imm,abs / imm,abs,X
		case 0x93:
		case 0xB3:
		{
			const uint16_t absAddr = state.ReadWord(pc + 2);
			pCodeInfo->OperandType = EOperandType::Hex;
			pCodeInfo->OperandAddress = FAddressRef::Invalid();
			pCodeInfo->ExtraOperands[0].Type = EOperandType::Pointer;
			pCodeInfo->ExtraOperands[0].Address = state.GetCanonicalAddressRef(absAddr);
			break;
		}
	}
#endif
}
