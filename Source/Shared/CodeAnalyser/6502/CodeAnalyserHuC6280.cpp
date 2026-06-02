#include "CodeAnalyserHuC6280.h"
#include "../CodeAnalyser.h"
#include <Debug/DebugLog.h>

enum class EAddressMode : uint8_t
{
	ZPIndirect_X,       // (zp,X)
	ZP,                 // zp
	Immediate,          // #
	Absolute,           // abs
	ZPIndirect_Y,       // (zp),Y
	ZP_X,               // zp,X
	Absolute_Y,         // abs,Y
	Absolute_X,         // abs,X
	Accumulator,        // A
	ZPIndirect,         // (zp)
	ZP_Y,               // zp,Y
	Relative,           // rel
	AbsoluteIndirect,   // (abs)
	AbsoluteIndirect_X, // (abs,X)
	ZPRelative,         // zp,rel  (BBR/BBS)
	Block,              // src,dst,len
	ImmZP,              // #,zp    (TST)
	ImmAbs,             // #,abs   (TST)
	ImmZPX,             // #,zp,X  (TST)
	ImmAbsX,            // #,abs,X (TST)
	Implied,
	Invalid,
	NA
};

static const char* g_AddrModeStrings[] =
{
	"(zp,X)",    // ZPIndirect_X
	"zp",        // ZP
	"#",         // Immediate
	"abs",       // Absolute
	"(zp),Y",    // ZPIndirect_Y
	"zp,X",      // ZP_X
	"abs,Y",     // Absolute_Y
	"abs,X",     // Absolute_X
	"A",         // Accumulator
	"(zp)",      // ZPIndirect
	"zp,Y",      // ZP_Y
	"rel",       // Relative
	"(abs)",     // AbsoluteIndirect
	"(abs,X)",   // AbsoluteIndirect_X
	"zp,rel",    // ZPRelative
	"block",     // Block
	"#,zp",      // ImmZP
	"#,abs",     // ImmAbs
	"#,zp,X",    // ImmZPX
	"#,abs,X",   // ImmAbsX
	"",          // Implied
	"???",       // Invalid
	"",          // NA
};

// One entry per opcode byte, derived from _huc6280dasm_ops[cc][bbb][aaa] in HuC6280Disassembler.cpp.
static const EAddressMode g_HuC6280AddrModes[256] =
{
	/* 00 BRK       */ EAddressMode::Implied,
	/* 01 ORA(zp,X) */ EAddressMode::ZPIndirect_X,
	/* 02 SXY       */ EAddressMode::Implied,
	/* 03 ST0 #     */ EAddressMode::Immediate,
	/* 04 TSB zp    */ EAddressMode::ZP,
	/* 05 ORA zp    */ EAddressMode::ZP,
	/* 06 ASL zp    */ EAddressMode::ZP,
	/* 07 RMB0 zp   */ EAddressMode::ZP,
	/* 08 PHP       */ EAddressMode::Implied,
	/* 09 ORA #     */ EAddressMode::Immediate,
	/* 0A ASL A     */ EAddressMode::Accumulator,
	/* 0B           */ EAddressMode::Invalid,
	/* 0C TSB abs   */ EAddressMode::Absolute,
	/* 0D ORA abs   */ EAddressMode::Absolute,
	/* 0E ASL abs   */ EAddressMode::Absolute,
	/* 0F BBR0      */ EAddressMode::ZPRelative,
	/* 10 BPL rel   */ EAddressMode::Relative,
	/* 11 ORA(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* 12 ORA (zp)  */ EAddressMode::ZPIndirect,
	/* 13 ST1 #     */ EAddressMode::Immediate,
	/* 14 TRB zp    */ EAddressMode::ZP,
	/* 15 ORA zp,X  */ EAddressMode::ZP_X,
	/* 16 ASL zp,X  */ EAddressMode::ZP_X,
	/* 17 RMB1 zp   */ EAddressMode::ZP,
	/* 18 CLC       */ EAddressMode::Implied,
	/* 19 ORA abs,Y */ EAddressMode::Absolute_Y,
	/* 1A INC A     */ EAddressMode::Implied,
	/* 1B           */ EAddressMode::Absolute_Y,
	/* 1C TRB abs   */ EAddressMode::Absolute,
	/* 1D ORA abs,X */ EAddressMode::Absolute_X,
	/* 1E ASL abs,X */ EAddressMode::Absolute_X,
	/* 1F BBR1      */ EAddressMode::ZPRelative,
	/* 20 JSR abs   */ EAddressMode::Absolute,
	/* 21 AND(zp,X) */ EAddressMode::ZPIndirect_X,
	/* 22 SAX       */ EAddressMode::Implied,
	/* 23 ST2 #     */ EAddressMode::Immediate,
	/* 24 BIT zp    */ EAddressMode::ZP,
	/* 25 AND zp    */ EAddressMode::ZP,
	/* 26 ROL zp    */ EAddressMode::ZP,
	/* 27 RMB2 zp   */ EAddressMode::ZP,
	/* 28 PLP       */ EAddressMode::Implied,
	/* 29 AND #     */ EAddressMode::Immediate,
	/* 2A ROL A     */ EAddressMode::Accumulator,
	/* 2B           */ EAddressMode::Invalid,
	/* 2C BIT abs   */ EAddressMode::Absolute,
	/* 2D AND abs   */ EAddressMode::Absolute,
	/* 2E ROL abs   */ EAddressMode::Absolute,
	/* 2F BBR2      */ EAddressMode::ZPRelative,
	/* 30 BMI rel   */ EAddressMode::Relative,
	/* 31 AND(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* 32 AND (zp)  */ EAddressMode::ZPIndirect,
	/* 33           */ EAddressMode::ZPIndirect_Y,
	/* 34 BIT zp,X  */ EAddressMode::ZP_X,
	/* 35 AND zp,X  */ EAddressMode::ZP_X,
	/* 36 ROL zp,X  */ EAddressMode::ZP_X,
	/* 37 RMB3 zp   */ EAddressMode::ZP,
	/* 38 SEC       */ EAddressMode::Implied,
	/* 39 AND abs,Y */ EAddressMode::Absolute_Y,
	/* 3A DEC A     */ EAddressMode::Implied,
	/* 3B           */ EAddressMode::Absolute_Y,
	/* 3C BIT abs,X */ EAddressMode::Absolute_X,
	/* 3D AND abs,X */ EAddressMode::Absolute_X,
	/* 3E ROL abs,X */ EAddressMode::Absolute_X,
	/* 3F BBR3      */ EAddressMode::ZPRelative,
	/* 40 RTI       */ EAddressMode::Implied,
	/* 41 EOR(zp,X) */ EAddressMode::ZPIndirect_X,
	/* 42 SAY       */ EAddressMode::Implied,
	/* 43 TMA #     */ EAddressMode::Immediate,
	/* 44 BSR rel   */ EAddressMode::Relative,
	/* 45 EOR zp    */ EAddressMode::ZP,
	/* 46 LSR zp    */ EAddressMode::ZP,
	/* 47 RMB4 zp   */ EAddressMode::ZP,
	/* 48 PHA       */ EAddressMode::Implied,
	/* 49 EOR #     */ EAddressMode::Immediate,
	/* 4A LSR A     */ EAddressMode::Accumulator,
	/* 4B           */ EAddressMode::Invalid,
	/* 4C JMP abs   */ EAddressMode::Absolute,
	/* 4D EOR abs   */ EAddressMode::Absolute,
	/* 4E LSR abs   */ EAddressMode::Absolute,
	/* 4F BBR4      */ EAddressMode::ZPRelative,
	/* 50 BVC rel   */ EAddressMode::Relative,
	/* 51 EOR(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* 52 EOR (zp)  */ EAddressMode::ZPIndirect,
	/* 53 TAM #     */ EAddressMode::Immediate,
	/* 54 CSL       */ EAddressMode::Implied,
	/* 55 EOR zp,X  */ EAddressMode::ZP_X,
	/* 56 LSR zp,X  */ EAddressMode::ZP_X,
	/* 57 RMB5 zp   */ EAddressMode::ZP,
	/* 58 CLI       */ EAddressMode::Implied,
	/* 59 EOR abs,Y */ EAddressMode::Absolute_Y,
	/* 5A PHY       */ EAddressMode::Implied,
	/* 5B           */ EAddressMode::Absolute_Y,
	/* 5C *NOP abs  */ EAddressMode::Absolute,
	/* 5D EOR abs,X */ EAddressMode::Absolute_X,
	/* 5E LSR abs,X */ EAddressMode::Absolute_X,
	/* 5F BBR5      */ EAddressMode::ZPRelative,
	/* 60 RTS       */ EAddressMode::Implied,
	/* 61 ADC(zp,X) */ EAddressMode::ZPIndirect_X,
	/* 62 CLA       */ EAddressMode::Implied,
	/* 63           */ EAddressMode::ZPIndirect_X,
	/* 64 STZ zp    */ EAddressMode::ZP,
	/* 65 ADC zp    */ EAddressMode::ZP,
	/* 66 ROR zp    */ EAddressMode::ZP,
	/* 67 RMB6 zp   */ EAddressMode::ZP,
	/* 68 PLA       */ EAddressMode::Implied,
	/* 69 ADC #     */ EAddressMode::Immediate,
	/* 6A ROR A     */ EAddressMode::Accumulator,
	/* 6B           */ EAddressMode::Invalid,
	/* 6C JMP (abs) */ EAddressMode::AbsoluteIndirect,
	/* 6D ADC abs   */ EAddressMode::Absolute,
	/* 6E ROR abs   */ EAddressMode::Absolute,
	/* 6F BBR6      */ EAddressMode::ZPRelative,
	/* 70 BVS rel   */ EAddressMode::Relative,
	/* 71 ADC(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* 72 ADC (zp)  */ EAddressMode::ZPIndirect,
	/* 73 TII block */ EAddressMode::Block,
	/* 74 STZ zp,X  */ EAddressMode::ZP_X,
	/* 75 ADC zp,X  */ EAddressMode::ZP_X,
	/* 76 ROR zp,X  */ EAddressMode::ZP_X,
	/* 77 RMB7 zp   */ EAddressMode::ZP,
	/* 78 SEI       */ EAddressMode::Implied,
	/* 79 ADC abs,Y */ EAddressMode::Absolute_Y,
	/* 7A PLY       */ EAddressMode::Implied,
	/* 7B           */ EAddressMode::Absolute_Y,
	/* 7C JMP(a,X)  */ EAddressMode::AbsoluteIndirect_X,
	/* 7D ADC abs,X */ EAddressMode::Absolute_X,
	/* 7E ROR abs,X */ EAddressMode::Absolute_X,
	/* 7F BBR7      */ EAddressMode::ZPRelative,
	/* 80 BRA rel   */ EAddressMode::Relative,
	/* 81 STA(zp,X) */ EAddressMode::ZPIndirect_X,
	/* 82 CLX       */ EAddressMode::Implied,
	/* 83 TST #,zp  */ EAddressMode::ImmZP,
	/* 84 STY zp    */ EAddressMode::ZP,
	/* 85 STA zp    */ EAddressMode::ZP,
	/* 86 STX zp    */ EAddressMode::ZP,
	/* 87 SMB0 zp   */ EAddressMode::ZP,
	/* 88 DEY       */ EAddressMode::Implied,
	/* 89 BIT #     */ EAddressMode::Immediate,
	/* 8A TXA       */ EAddressMode::Implied,
	/* 8B           */ EAddressMode::Invalid,
	/* 8C STY abs   */ EAddressMode::Absolute,
	/* 8D STA abs   */ EAddressMode::Absolute,
	/* 8E STX abs   */ EAddressMode::Absolute,
	/* 8F BBS0      */ EAddressMode::ZPRelative,
	/* 90 BCC rel   */ EAddressMode::Relative,
	/* 91 STA(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* 92 STA (zp)  */ EAddressMode::ZPIndirect,
	/* 93 TST #,abs */ EAddressMode::ImmAbs,
	/* 94 STY zp,X  */ EAddressMode::ZP_X,
	/* 95 STA zp,X  */ EAddressMode::ZP_X,
	/* 96 STX zp,Y  */ EAddressMode::ZP_Y,
	/* 97 SMB1 zp   */ EAddressMode::ZP,
	/* 98 TYA       */ EAddressMode::Implied,
	/* 99 STA abs,Y */ EAddressMode::Absolute_Y,
	/* 9A TXS       */ EAddressMode::Implied,
	/* 9B           */ EAddressMode::Invalid,
	/* 9C STZ abs   */ EAddressMode::Absolute,
	/* 9D STA abs,X */ EAddressMode::Absolute_X,
	/* 9E STZ abs,X */ EAddressMode::Absolute_X,
	/* 9F BBS1      */ EAddressMode::ZPRelative,
	/* A0 LDY #     */ EAddressMode::Immediate,
	/* A1 LDA(zp,X) */ EAddressMode::ZPIndirect_X,
	/* A2 LDX #     */ EAddressMode::Immediate,
	/* A3 TST #,z,X */ EAddressMode::ImmZPX,
	/* A4 LDY zp    */ EAddressMode::ZP,
	/* A5 LDA zp    */ EAddressMode::ZP,
	/* A6 LDX zp    */ EAddressMode::ZP,
	/* A7 SMB2 zp   */ EAddressMode::ZP,
	/* A8 TAY       */ EAddressMode::Implied,
	/* A9 LDA #     */ EAddressMode::Immediate,
	/* AA TAX       */ EAddressMode::Implied,
	/* AB           */ EAddressMode::Invalid,
	/* AC LDY abs   */ EAddressMode::Absolute,
	/* AD LDA abs   */ EAddressMode::Absolute,
	/* AE LDX abs   */ EAddressMode::Absolute,
	/* AF BBS2      */ EAddressMode::ZPRelative,
	/* B0 BCS rel   */ EAddressMode::Relative,
	/* B1 LDA(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* B2 LDA (zp)  */ EAddressMode::ZPIndirect,
	/* B3 TST #,a,X */ EAddressMode::ImmAbsX,
	/* B4 LDY zp,X  */ EAddressMode::ZP_X,
	/* B5 LDA zp,X  */ EAddressMode::ZP_X,
	/* B6 LDX zp,Y  */ EAddressMode::ZP_Y,
	/* B7 SMB3 zp   */ EAddressMode::ZP,
	/* B8 CLV       */ EAddressMode::Implied,
	/* B9 LDA abs,Y */ EAddressMode::Absolute_Y,
	/* BA TSX       */ EAddressMode::Implied,
	/* BB           */ EAddressMode::Invalid,
	/* BC LDY abs,X */ EAddressMode::Absolute_X,
	/* BD LDA abs,X */ EAddressMode::Absolute_X,
	/* BE LDX abs,Y */ EAddressMode::Absolute_Y,
	/* BF BBS3      */ EAddressMode::ZPRelative,
	/* C0 CPY #     */ EAddressMode::Immediate,
	/* C1 CMP(zp,X) */ EAddressMode::ZPIndirect_X,
	/* C2 CLY       */ EAddressMode::Implied,
	/* C3 TDD block */ EAddressMode::Block,
	/* C4 CPY zp    */ EAddressMode::ZP,
	/* C5 CMP zp    */ EAddressMode::ZP,
	/* C6 DEC zp    */ EAddressMode::ZP,
	/* C7 SMB4 zp   */ EAddressMode::ZP,
	/* C8 INY       */ EAddressMode::Implied,
	/* C9 CMP #     */ EAddressMode::Immediate,
	/* CA DEX       */ EAddressMode::Implied,
	/* CB           */ EAddressMode::Invalid,
	/* CC CPY abs   */ EAddressMode::Absolute,
	/* CD CMP abs   */ EAddressMode::Absolute,
	/* CE DEC abs   */ EAddressMode::Absolute,
	/* CF BBS4      */ EAddressMode::ZPRelative,
	/* D0 BNE rel   */ EAddressMode::Relative,
	/* D1 CMP(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* D2 CMP (zp)  */ EAddressMode::ZPIndirect,
	/* D3 TIN block */ EAddressMode::Block,
	/* D4 CSH       */ EAddressMode::Implied,
	/* D5 CMP zp,X  */ EAddressMode::ZP_X,
	/* D6 DEC zp,X  */ EAddressMode::ZP_X,
	/* D7 SMB5 zp   */ EAddressMode::ZP,
	/* D8 CLD       */ EAddressMode::Implied,
	/* D9 CMP abs,Y */ EAddressMode::Absolute_Y,
	/* DA PHX       */ EAddressMode::Implied,
	/* DB           */ EAddressMode::Absolute_Y,
	/* DC *NOP abs,X*/ EAddressMode::Absolute_X,
	/* DD CMP abs,X */ EAddressMode::Absolute_X,
	/* DE DEC abs,X */ EAddressMode::Absolute_X,
	/* DF BBS5      */ EAddressMode::ZPRelative,
	/* E0 CPX #     */ EAddressMode::Immediate,
	/* E1 SBC(zp,X) */ EAddressMode::ZPIndirect_X,
	/* E2 *NOP #    */ EAddressMode::Immediate,
	/* E3 TIA block */ EAddressMode::Block,
	/* E4 CPX zp    */ EAddressMode::ZP,
	/* E5 SBC zp    */ EAddressMode::ZP,
	/* E6 INC zp    */ EAddressMode::ZP,
	/* E7 SMB6 zp   */ EAddressMode::ZP,
	/* E8 INX       */ EAddressMode::Implied,
	/* E9 SBC #     */ EAddressMode::Immediate,
	/* EA NOP       */ EAddressMode::Implied,
	/* EB *SBC #    */ EAddressMode::Immediate,
	/* EC CPX abs   */ EAddressMode::Absolute,
	/* ED SBC abs   */ EAddressMode::Absolute,
	/* EE INC abs   */ EAddressMode::Absolute,
	/* EF BBS6      */ EAddressMode::ZPRelative,
	/* F0 BEQ rel   */ EAddressMode::Relative,
	/* F1 SBC(zp),Y */ EAddressMode::ZPIndirect_Y,
	/* F2 SBC (zp)  */ EAddressMode::ZPIndirect,
	/* F3 TAI block */ EAddressMode::Block,
	/* F4 SET       */ EAddressMode::Implied,
	/* F5 SBC zp,X  */ EAddressMode::ZP_X,
	/* F6 INC zp,X  */ EAddressMode::ZP_X,
	/* F7 SMB7 zp   */ EAddressMode::ZP,
	/* F8 SED       */ EAddressMode::Implied,
	/* F9 SBC abs,Y */ EAddressMode::Absolute_Y,
	/* FA PLX       */ EAddressMode::Implied,
	/* FB           */ EAddressMode::Absolute_Y,
	/* FC *NOP abs,X*/ EAddressMode::Absolute_X,
	/* FD SBC abs,X */ EAddressMode::Absolute_X,
	/* FE INC abs,X */ EAddressMode::Absolute_X,
	/* FF BBS7      */ EAddressMode::ZPRelative,
};

EAddressMode GetInstructionAddressModeHuC6280(uint8_t opcode)
{
	return g_HuC6280AddrModes[opcode];
}

const char* GetAddressModeStringHuC6280(uint8_t opcode)
{
	return g_AddrModeStrings[(int)g_HuC6280AddrModes[opcode]];
}

// Returns true if the instruction at pc reads/writes a memory address, outputting that address.
// The address is used to generate a data label at the target location.
bool CheckPointerIndirectionInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
	const uint8_t instrByte = state.ReadByte(pc);
	const EAddressMode addrMode = GetInstructionAddressModeHuC6280(instrByte);

	switch (addrMode)
	{
	case EAddressMode::ZPIndirect_X:
	case EAddressMode::ZPIndirect_Y:
	case EAddressMode::ZP:
	case EAddressMode::ZP_X:
	case EAddressMode::ZP_Y:
	case EAddressMode::ZPIndirect:
		*out_addr = 0x2000 + state.ReadByte(pc + 1);
		return true;

	case EAddressMode::Absolute:
	case EAddressMode::Absolute_X:
	case EAddressMode::Absolute_Y:
		*out_addr = state.ReadWord(pc + 1);
		return true;

	default:
		return false;
	}
}

// Returns true if the instruction at pc references a memory address, outputting that address.
// Unlike CheckPointerIndirectionInstruction, no data label is generated at the target location.
bool CheckPointerRefInstructionHuC6280(const FCodeAnalysisState& state, uint16_t pc, uint16_t* out_addr)
{
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

// sam. currently disabled. asm export is broken with this enabled.
// also see ALLOW_U8_PTRS_JUMPS in Disassembler.cpp
#define MULTIPLE_OPERANDS_SUPPORT 1

void FillCodeInfoOperandsHuC6280(FCodeAnalysisState& state, uint16_t pc, FCodeInfo* pCodeInfo)
{
	const FAddressRef pcAddrRef = state.GetCanonicalAddressRef(pc);
	const uint8_t instrByte = state.ReadByte(pc);

//#if MULTIPLE_OPERANDS_SUPPORT
	// BBR/BBS: 3-byte (opcode, zp, rel). Operand 0 = ZP pointer, operand 1 = branch target.
	// Handled before standard jump path, which would otherwise overwrite operand 0.
	if ((instrByte & 0x0F) == 0x0F)
	{
		const uint8_t zpAddr = state.ReadByte(pc + 1);
		const int8_t relJump = (int8_t)state.ReadByte(pc + 2);
		const uint16_t branchTarget = pc + 3 + relJump;
		const FAddressRef zpAddrRef = state.GetCanonicalAddressRef(0x2000 + zpAddr);
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
//#endif

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
	}
#if MULTIPLE_OPERANDS_SUPPORT
	// HuC6280 multi-operand instructions
	switch (instrByte)
	{
		// TST imm,zp / imm,zp,X
		case 0x83:
		case 0xA3:
		{
			const uint16_t zpAddr = state.ReadByte(pc + 2);
			pCodeInfo->OperandType = EOperandType::Hex;
			pCodeInfo->OperandAddress = FAddressRef::Invalid();
			pCodeInfo->ExtraOperands[0].Type = EOperandType::Pointer;
			pCodeInfo->ExtraOperands[0].Address = state.GetCanonicalAddressRef(0x2000 + zpAddr);
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
