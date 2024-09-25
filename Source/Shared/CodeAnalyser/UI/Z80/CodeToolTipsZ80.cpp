#include "CodeToolTipsZ80.h"
#include "../CodeToolTips.h"
#include "../../CodeAnalyser.h"
#include "../CodeAnalyserUI.h"
#include "RegisterViewZ80.h"

#include <Util/Misc.h>
#include <chips/z80.h>

#include "imgui.h"

#include <map>
#include <deque>

namespace Z80ToolTip
{
// Don't change the order of these enums.
enum class EOperand
{
	None = -1,

	// 8 bit
	A = 0,
	B,
	C,
	D,
	E,
	H,
	L,
	I,
	R,
	IXL,
	IXH,
	IYL,
	IYH,
	F,

	// 16 bit registers
	AF,
	AF_ALT,
	BC,
	DE,
	HL,
	IX,
	IY,
	SP,

	// (BC), (DE), etc
	BC_Indirect,
	DE_Indirect,
	HL_Indirect,
	IX_Indirect,
	IY_Indirect,
	SP_Indirect,

	// Indexed addressing. (IX + d), (IY + d).
	IX_Indirect_D,
	IY_Indirect_D,

	// Indirect memory addressed by immediate data.
	// Used in "LD (nn),r" "LD r,(nn)", "LD rr,(nn)" etc
	Indirect_Immediate_Word, // Word value at address
	Indirect_Immediate_Byte, // Byte value at address
};

struct FOperand
{
	FOperand()
	{
	}
	FOperand(EOperand type, ENumberDisplayMode displayMode = ENumberDisplayMode::None, int8_t displacement = 0, uint16_t address = 0)
		: Type(type),
		Displacement(displacement),
		DisplayMode(displayMode),
		Address(address) 
	{
	}
	bool IsByteOperand() const
	{
		return Type <= EOperand::F;
	}
	bool IsAddress() const
	{
		// 16 bit operands that are not memory locations will return false from here.
		return !IsByteOperand() && Type != EOperand::AF && Type != EOperand::AF_ALT && DisplayMode != ENumberDisplayMode::Decimal;
	}

	EOperand Type = EOperand::None;
	ENumberDisplayMode DisplayMode = ENumberDisplayMode::None;
	
	int8_t Displacement = 0;	// Displacement. For (IX + d) and (IY + d) only
	uint16_t Address = 0;			// For indirect immediate operands.
};

typedef std::vector<FOperand*> TOperandList;

struct FInstruction
{
	std::string Title;
	std::string Description;
	std::deque<FOperand> Operands;
};

std::string GetOperandName(EOperand operand, int8_t displacement = 0, uint16_t address = 0);

static EOperand g_TTOperand[8] = { EOperand::B, EOperand::C, EOperand::D, EOperand::E, EOperand::H, EOperand::L, EOperand::HL_Indirect, EOperand::A };
static EOperand g_TTOperandIX[8] = { EOperand::B, EOperand::C, EOperand::D, EOperand::E, EOperand::IXH, EOperand::IXL, EOperand::IX_Indirect_D, EOperand::A };
static EOperand g_TTOperandIY[8] = { EOperand::B, EOperand::C, EOperand::D, EOperand::E, EOperand::IYH, EOperand::IYL, EOperand::IY_Indirect_D, EOperand::A };

static EOperand g_TTOperandPairs[4] = { EOperand::BC, EOperand::DE, EOperand::HL, EOperand::SP };
static EOperand g_TTOperandPairsIX[4] = { EOperand::BC, EOperand::DE, EOperand::IX, EOperand::SP };
static EOperand g_TTOperandPairsIY[4] = { EOperand::BC, EOperand::DE, EOperand::IY, EOperand::SP };

static EOperand g_TTOperandPairs2[4] = { EOperand::BC, EOperand::DE, EOperand::HL, EOperand::AF };
static EOperand g_TTOperandPairs2IX[4] = { EOperand::BC, EOperand::DE, EOperand::IX, EOperand::AF };
static EOperand g_TTOperandPairs2IY[4] = { EOperand::BC, EOperand::DE, EOperand::IY, EOperand::AF };


const int kTTZ80DescLen = 512;
const int kTTZ80TitleLen = 128;
static char g_TTZ80DescBuf[kTTZ80DescLen];
static char g_TTZ80TitleBuf[kTTZ80TitleLen];

void Do8BitLoadBlock(
	uint8_t y, uint8_t z, uint8_t prefix, EOperand* r, uint16_t pc, const FCodeAnalysisState& state, FInstruction& inst)
{
	int8_t d = 0;
	if (prefix)
	{
		// displacement
		d = state.ReadByte(pc);
	}

	if (y == 6)
	{
		if (z == 6)
		{
			/* special case LD (HL),(HL) */
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Suspend CPU operation until interrupt or reset occurs.");
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Halt CPU");
		}
		else
		{
			/* LD (HL),r; LD (IX+d),r; LD (IY+d),r */
			if (prefix && ((z == 4) || (z == 5)))
			{
				/* special case LD (IX+d),L/H (don't use IXL/IXH) */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetOperandName(r[6], d).c_str(), GetOperandName(g_TTOperand[z]).c_str());
				inst.Operands = { FOperand(r[6], ENumberDisplayMode::None, d), FOperand(g_TTOperand[z]) };
			}
			else
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetOperandName(r[6], d).c_str(), GetOperandName(r[z]).c_str());
				inst.Operands = { FOperand(r[6], ENumberDisplayMode::None, d), FOperand(r[z]) };
			}
		}
	}
	else if (z == 6)
	{
		/* LD r,(HL); LD r,(IX+d); LD r,(IY+d) */
		if (prefix && ((y == 4) || (y == 5)))
		{
			/* special case LD H/L,(IX+d) (don't use IXL/IXH) */
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetOperandName(g_TTOperand[y]).c_str(), GetOperandName(r[6], d).c_str());
			inst.Operands = { FOperand(g_TTOperand[y]), FOperand(r[6], ENumberDisplayMode::None, d) };
		}
		else
		{
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetOperandName(r[y]).c_str(), GetOperandName(r[6], d).c_str());
			inst.Operands = { FOperand(r[y]), FOperand(r[6], ENumberDisplayMode::None, d) };
		}
	}
	else
	{
		/* regular LD r,s */
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetOperandName(r[y]).c_str(), GetOperandName(r[z]).c_str());
		inst.Operands = { FOperand(g_TTOperand[y]), FOperand(g_TTOperand[z]) };
	}
}

void Do8BitALUBlock(uint8_t y, uint8_t z, uint8_t prefix, EOperand* r, uint16_t pc, const FCodeAnalysisState& state, FInstruction& inst)
{
	int8_t d = 0;
	if (prefix)
	{
		d = state.ReadByte(pc);
	}

	const std::string regName = GetOperandName(r[z], d);
	bool bBinary = false;

	switch (y)
	{
	case 0: // ADD A, r 
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Add %s to A. Result stored in A.", regName.c_str());
		break;
	case 1: // ADC A, s 
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Add %s and C flag to A. Result stored in A.", regName.c_str());
		break;
	case 2: // SUB s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract %s from A. Result stored in A.", regName.c_str());
		break;
	case 3: // SBC A, s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract %s and C flag from A. Result stored in A.", regName.c_str());
		break;
	case 4: // AND s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Logical AND A with %s. Result stored in A.", regName.c_str());
		bBinary = true;
		break;
	case 5: // XOR s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Exclusive OR A with %s. Result stored in A.", regName.c_str());
		bBinary = true;
		break;
	case 6: // OR s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Logical OR A with %s. Result stored in A.", regName.c_str());
		bBinary = true;
		break;
	case 7: // CP s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract %s from A but discard result.\n\n"
			"Unsigned\nA == %s: Z flag is set.\nA != %s: Z flag is reset.\n"
			"A < %s:  C flag is set.\nA >= %s: C flag is reset.\n\n"
			"Signed\nA == %s: Z flag is set.\nA != %s: Z flag is reset.\n"
			"A < %s:  S and P/V are different.\nA >= %s: S and P/V are the same.\n",
			regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str());
		snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Compare A with %s", regName.c_str());
		break;
	}

	ENumberDisplayMode numMode = bBinary ? ENumberDisplayMode::Binary : ENumberDisplayMode::None;

	inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A, numMode), FOperand(r[z], numMode, d) };
}

void DoCBPrefix(uint8_t prefix, EOperand* r, uint16_t pc, const FCodeAnalysisState& state, FInstruction& inst)
{
	int8_t d = 0;
	uint8_t op = 0;

	if (prefix)
		d = state.ReadByte(pc++);
	op = state.ReadByte(pc++);

	const uint8_t x = (op >> 6) & 3;
	const uint8_t y = (op >> 3) & 7;
	const uint8_t z = op & 7;

	const std::string regName = GetOperandName(r[z], d);

	inst.Operands = { FOperand(r[z], ENumberDisplayMode::Binary, d) };

	if (x == 0)
	{
		/* rot and shift instructions */
		/* RLC, RRC, RL, RR, SLA, SRA, SLL, SRL */
		switch (y)
		{
		case 0: // RLC 
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Rotate %s left 1 bit. Bit 7 is copied to C flag and also to bit 0.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate left %s with branch carry", regName.c_str());
			break;
		case 1: // RRC
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Rotate %s right 1 bit. Bit 7 is copied to C flag and also to bit 0.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate right %s with branch carry", regName.c_str());
			break;
		case 2: // RL 
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Rotate %s left one bit. Bit 7 is copied to C flag and previous C flag copied to bit 0.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate left %s through carry", regName.c_str());
			break;
		case 3: // RR
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Rotate %s right one bit. Bit 0 is copied to C flag and previous C flag copied to bit 7.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate right %s through carry", regName.c_str());
			break;
		case 4: // SLA s
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Arithmetic shift left %s one bit. Bit 7 is copied to the C flag and bit 0 is reset.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Arithmetic shift left %s", regName.c_str());
			break;
		case 5: // SRA s
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Arithmetic shift right %s one bit. Bit 0 is copied to C flag and the previous contents of bit 7 remain unchanged.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Arithmetic shift right %s", regName.c_str());
			break;
		case 6: // SLL - Undocumented
			//snprintf(tempStr, kTempStrLen, "SLL %s", regName.c_str());
			break;
		case 7: // SRL s
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Rotate %s right one bit. Bit 0 is copied to the C flag and bit 7 is reset.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Logical shift right %s", regName.c_str());
			break;
		}

		inst.Operands.push_front(FOperand(EOperand::F));
	}
	else if (x == 1) /* BIT b, r */
	{
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Z flag is set if bit %d of %s is 0; otherwise Z flag is reset.", y, regName.c_str());
		snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Test bit %d of %s", y, regName.c_str());

	}
	else if (x == 2) /* RES b, s */
	{
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Reset bit %d of %s.", y, regName.c_str());
	}
	else if (x == 3) /* SET b, s */
	{
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Set bit %d of %s.", y, regName.c_str());
	}
}

void DoEDPrefix(uint8_t prefix, EOperand* r, EOperand* rp, uint16_t pc, const FCodeAnalysisState& state, FInstruction& inst)
{
	z80_t* pCPU = (z80_t*)state.GetCPUInterface()->GetCPUEmulator();
	const uint8_t op = state.ReadByte(pc++);
	const uint8_t x = (op >> 6) & 3;
	const uint8_t y = (op >> 3) & 7;
	const uint8_t z = op & 7;
	const uint8_t p = y >> 1;
	const uint8_t q = y & 1;

	if ((x == 0) || (x == 3))
	{
		/* NOP (ED)*/
	}
	else if (x == 2)
	{
		if ((y >= 4) && (z <= 3))
		{
			/* block instructions */
			if (z == 0) /* First column. LDI, LDD, LDIR, LDDR*/
			{
				switch (y - 4)
				{
				case 0: /* LDI */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is loaded into (DE) and both DE and HL are incremented, and BC is decremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block load with increment");
					break;
				case 1: /* LDD */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is loaded into (DE) then BC, DE and HL are all decremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block load with decrement");
					break;
				case 2: /* LDIR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is loaded into (DE) then DE and HL are incremented, and BC is decremented. "
						"If BC is not 0 then the instruction is repeated.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Repeating Block Load With Increment.");
					break;
				case 3: /* LDDR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is loaded into (DE). Then BC, DE and HL are all decremented. "
						"If BC is not 0 then the instruction is repeated.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Repeating block load with decrement");
					break;
				}
				inst.Operands = { FOperand(EOperand::DE_Indirect), FOperand(EOperand::HL_Indirect), FOperand(EOperand::BC, ENumberDisplayMode::Decimal) };
			}
			else if (z == 1) /* Second column. CPI, CPD, CPIR, CPDR*/
			{
				switch (y - 4)
				{
				case 0: /* CPI */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is compared with A. If they are equal the Z flag is set. "
						"Then HL is incremented and BC is decremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Compare with increment");
					break;
				case 1: /* CPD */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is compared with A. If they are equal the Z flag is set. HL and BC are decremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Compare with decrement");
					break;
				case 2: /* CPIR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is compared with A, HL is incremented and BC decremented. "
						"The instruction is terminated if BC becomes 0 or if A equals (HL). "
						"If the value is found, HL will point to the address after the found byte and the Z flag will be set.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block compare with increment");
					break;
				case 3: /* CPDR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is compared with A. Then HL is incremented and BC is decremented. "
						"If BC becomes 0 or if (HL) equals A then the instruction is terminated, otherwise the instruction is repeated.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block compare with decrement");
					break;
				}
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A), FOperand(EOperand::HL) };
			}
			else if (z == 2) /* Third column. INI, IND, INIR, INDR */
			{
				switch (y - 4)
				{
				case 0: /* INI */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (C) is read and the result is loaded into (HL). "
						"B is decremented and HL is incremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Input with increment");
					break;
				case 1: /* IND */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (C) is read and the result is loaded into (HL). "
						"B and HL are then both decremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Input with decrement");
					break;
				case 2: /* INIR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (C) is read and the result is loaded into (HL). "
						"B is decremented and HL is incremented. If B is not zero the instruction is re-executed");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block input with increment");
					break;
				case 3: /* INDR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (C) is read and the result is loaded into (HL). "
						"B and HL are then both decremented. If B is not zero the instruction is re-executed");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block input with decrement");
					break;
				}
				inst.Operands = { FOperand(EOperand::B, ENumberDisplayMode::Decimal), FOperand(EOperand::C), FOperand(EOperand::HL_Indirect)  };
			}
			else /* Fourth column. OUTI, OUTD, OTIR, OTDR */
			{
				switch (y - 4)
				{
				case 0: /* OUTI */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is written to the port (C). HL is incremented and B is decremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Output with increment");
					break;
				case 1: /* OUTD */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is written to port (C). HL and B are both decremented.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Output with decrement");
					break;
				case 2: /* OTIR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is written to port (C). HL is incremented and B is decremented. Repeats until B is 0.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block output with increment");
					break;
				case 3: /* OTDR */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(HL) is written to port (C). HL and B are both decremented. Repeats until B is 0");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Block output with decrement");
					break;
				}
				inst.Operands = { FOperand(EOperand::B, ENumberDisplayMode::Decimal), FOperand(EOperand::C), FOperand(EOperand::HL_Indirect) };
			}
		}
		else
		{
			/* NOP (ED)*/
		}
	}
	else
	{
		switch (z)
		{
		case 0: /* IN r, (C)*/
			if (y != 6)
			{
				const std::string strReg = GetOperandName(r[y]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (C) is read and the result is loaded into %s.", strReg.c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Load %s from port (C)", strReg.c_str());
				inst.Operands = { FOperand(r[y]), FOperand(EOperand::BC) };
			}
			break;
		case 1: /* OUT (C), r*/
			if (y != 6)
			{
				const std::string strReg = GetOperandName(r[y]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "The contents of %s is written to port (C).", strReg.c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Output %s to port (C)", strReg.c_str());
				inst.Operands = { FOperand(r[y]), FOperand(EOperand::BC) };
			}
			else // OUT (C), 0 [Undocumented]
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "0 is written to port (C).");
				inst.Operands = { FOperand(EOperand::BC) };
			}
			break;
		case 2: /* SBC HL, ss. ADC HL, ss. Where ss is BC/DE/HL/SP*/
			if (q == 0) // SBC HL, ss 
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s and C flag are subtracted from HL. Result stored in HL.", GetOperandName(rp[p]).c_str());
			}
			else // ADC HL, ss
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s and C flag are added to HL. Result stored in HL.", GetOperandName(rp[p]).c_str());
			}
			inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::HL), FOperand(rp[p]) };
			break;
		case 3: /* LD (nn), dd. LD dd, (nn). */
		{
			const uint16_t nn = state.ReadWord(pc);
			if (q == 0) // LD (nn), dd
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load memory locations (%s) and (%s) from %s.", NumStr(nn), NumStr(uint16_t(nn + 1)), GetOperandName(rp[p]).c_str());
				inst.Operands = { FOperand(EOperand::Indirect_Immediate_Word, ENumberDisplayMode::None, 0, nn), FOperand(rp[p]) };
			}
			else // LD dd, (nn)
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from memory locations (%s) and (%s).", GetOperandName(rp[p]).c_str(), NumStr(nn), NumStr(uint16_t(nn + 1)));
				inst.Operands = { FOperand(rp[p]), FOperand(EOperand::Indirect_Immediate_Word, ENumberDisplayMode::None, 0, nn) };
			}
			break;
		}
		case 4: /* NEG */
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Negate A (two's complement). Same result as subtracting A from 0.");
			inst.Operands = { FOperand(EOperand::A) };
			break;
		case 5: /* RETI/RETN*/
		{
			if (y == 0)
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "PC is popped off the stack."
					"IFF2 is copied back to IFF1 to restore the state of the "
					"interrupt flag before the non-maskable interrupt.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Return from non-maskable interrupt");
			}
			else
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "PC is popped off the stack.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Return from interrupt");
			}
			inst.Operands = { FOperand(EOperand::SP_Indirect) };
			break;
		}
		case 6: /* IM 0/1/2*/
		{
			const uint8_t im[8] = { 0, 0, 1, 2, 0, 0, 1, 2 };
			switch (im[y])
			{
			case 0:
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "In this mode the interrupting device may insert one instruction onto the data bus for execution, the first byte of which must occur during the interrupt acknowledge cycle.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Set interrupt mode 0");
				break;
			case 1:
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "In this mode the CPU responds to an interrupt by executing a restart at address %s", NumStr(uint16_t(0x38)));
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Set interrupt mode 1");
				break;
			case 2:
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "This mode allows an indirect call to any memory location by an 8-bit vector supplied from the peripheral device. This vector then becomes the least-significant eight bits of the indirect pointer, while the I Register in the CPU provides the most-significant eight bits. This address points to an address in a vector table that is the starting address for the interrupt service routine.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Set interrupt mode 2");
				break;
			}
			break;
		}
		case 7: /* LD I,A, LD R,A, LD A,I, LD A,R, RRD, RLD, NOP (ED), NOP (ED)*/
			switch (y)
			{
			case 0: /* LD I,A */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load Interrupt Vector Register I from A.");
				inst.Operands = { FOperand(EOperand::I), FOperand(EOperand::A) };
				break;
			case 1: /* LD R,A */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load Memory Refresh register R from A.");
				inst.Operands = { FOperand(EOperand::R), FOperand(EOperand::A) };
				break;
			case 2: /* LD A,I */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A from Interrupt Vector Register I.");
				inst.Operands = { FOperand(EOperand::A), FOperand(EOperand::I) };
				break;
			case 3: /* LD A,R */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A from Memory Refresh Register R.");
				inst.Operands = { FOperand(EOperand::A), FOperand(EOperand::R) };
				break;
			case 4: /* RRD */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Upper nibble of (HL) moved to lower nibble of (HL). Lower nibble of (HL) moved to lower nibble of A. Lower nibble of A moved to upper nibble of (HL). All operations occur simultaneously.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate right decimal");
				inst.Operands = { FOperand(EOperand::A, ENumberDisplayMode::Binary), FOperand(EOperand::HL_Indirect) };
				break;
			case 5: /* RLD */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Lower nibble of (HL) moved to lower nibble of (HL). Upper nibble of (HL) moved to lower nibble of A. Lower nibble of A moved to (HL). All operations occur simultaneously.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate left decimal");
				inst.Operands = { FOperand(EOperand::A, ENumberDisplayMode::Binary), FOperand(EOperand::HL_Indirect) };
				break;
			case 6:
				// NOP (ED)
				break;
			case 7:
				// NOP (ED)
				break;
			}
			break;
		}
	}
}

// Reference:
// http://www.z80.info/decoding.htm
void GetInstructionInfo(uint16_t pc, const FCodeAnalysisState& state, FInstruction& inst)
{
	uint8_t op = 0, prefix = 0;

	EOperand* r = g_TTOperand;
	EOperand* rp = g_TTOperandPairs;
	EOperand* rp2 = g_TTOperandPairs2;

	z80_t* pCPU = (z80_t*)state.GetCPUInterface()->GetCPUEmulator();

	g_TTZ80DescBuf[0] = 0;
	g_TTZ80TitleBuf[0] = 0;

	/* fetch the first instruction byte */
	op = state.ReadByte(pc++);

	/* prefixed op? */
	if ((0xFD == op) || (0xDD == op))
	{
		prefix = op;
		op = state.ReadByte(pc++);

		if (op == 0xED)
		{
			prefix = 0; /* an ED following a prefix cancels the prefix */
		}

		/* if prefixed op, use register tables that replace HL with IX/IY */
		if (prefix == 0xDD)
		{
			r = g_TTOperandIX;
			rp = g_TTOperandPairsIX;
			rp2 = g_TTOperandPairs2IX;
		}
		else if (prefix == 0xFD)
		{
			r = g_TTOperandIY;
			rp = g_TTOperandPairsIY;
			rp2 = g_TTOperandPairs2IY;
		}
	}

	/* parse the opcode */
	uint8_t x = (op >> 6) & 3;	// top 2 bits: 6 & 7
	uint8_t y = (op >> 3) & 7;	// "middle" 3 bits: 3,4 & 5
	uint8_t z = op & 7;			// lower 3 bits: 0,1 & 2
	uint8_t p = y >> 1;			// bits 4 & 5
	uint8_t q = y & 1;			// bit 3
	if (x == 1)
	{
		/* 8-bit load block */
		Do8BitLoadBlock(y, z, prefix, r, pc, state, inst);
	}
	else if (x == 2)
	{
		/* 8-bit ALU block */
		/* ADD A,s, ADC A,s, SUB s, SBC A,s, AND s, XOR s, OR s, CP s */
		/* Where s is (HL)/(IX+d)/(IX+d) or r */
		Do8BitALUBlock(y, z, prefix, r, pc, state, inst);
	}
	else if (x == 0)
	{
		switch (z)
		{
		case 0:
			switch (y)
			{
			case 0: /* NOP */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Nothing is done for 1 machine cycle.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "No operation");
				break;
			case 1: /* EX AF,AF'*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Swap AF and AF'.");
				inst.Operands = { FOperand(EOperand::AF), FOperand(EOperand::AF_ALT) };
				break;
			case 2: /* DJNZ*/
			{
				const int8_t offset = state.ReadByte(pc);
				const uint16_t addr = pc + offset + 1;
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement B and jump %s to %s on no zero.", offset < 0 ? "back" : "forward", NumStr(addr));
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::B, ENumberDisplayMode::Decimal) };
				break;
			}
			case 3: /* JR e*/
			{
				const int8_t offset = state.ReadByte(pc);
				const uint16_t addr = pc + offset + 1;
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump %s to %s.", offset < 0 ? "back" : "forward", NumStr(addr));
				break;
			}
			default: /* JR cc, e*/
			{
				const int8_t offset = state.ReadByte(pc);
				const uint16_t addr = pc + offset + 1;

				switch (y - 4)
				{
				case 0: /* NZ */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump %s to %s if the Z flag is not set.", offset < 0 ? "back" : "forward", NumStr(addr));
					break;
				case 1: /* Z */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump %s to %s if the Z flag is set.", offset < 0 ? "back" : "forward", NumStr(addr));
					break;
				case 2: /* NC */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump %s to %s if the C flag is not set.", offset < 0 ? "back" : "forward", NumStr(addr));
					break;
				case 3: /* C */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump %s to %s if the C flag is set.", offset < 0 ? "back" : "forward", NumStr(addr));
					break;
				}
				inst.Operands = { FOperand(EOperand::F) };
				break;
			}
			}
			break;
		case 1:

			if (q == 0) /* LD dd,nn*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s with immediate data %s.", GetOperandName(rp[p]).c_str(), NumStr(nn));
				inst.Operands = { FOperand(rp[p]), FOperand(EOperand::Indirect_Immediate_Word, ENumberDisplayMode::None, 0, nn) };
			}
			else /* ADD HL, ss. ADD IX, pp. ADD IY, rr. */
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "ADD %s to %s and store result in %s.", GetOperandName(rp[2]).c_str(), GetOperandName(rp[p]).c_str(), GetOperandName(rp[2]).c_str());
				inst.Operands = { FOperand(EOperand::F), FOperand(rp2[2]), FOperand(rp[p]) };
			}
			break;
		case 2:
		{
			switch (y)
			{
			case 0: /* LD (BC),A */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (BC) with A.");
				inst.Operands = { FOperand(EOperand::BC_Indirect), FOperand(EOperand::A) };
				break;
			case 1: /* LD A,(BC) */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A with (BC).");
				inst.Operands = { FOperand(EOperand::A), FOperand(EOperand::BC_Indirect) };
				break;
			case 2: /* LD (DE),A*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (DE) with A.");
				inst.Operands = { FOperand(EOperand::DE_Indirect), FOperand(EOperand::A) };
				break;
			case 3: /* LD A,(DE)*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A with (DE).");
				inst.Operands = { FOperand(EOperand::A), FOperand(EOperand::DE_Indirect) };
				break;
			case 4: /* LD (nn), HL/IX/IY*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load the memory locations %s and %s from %s.", NumStr(nn), NumStr(uint16_t(nn + 1)), GetOperandName(rp[p]).c_str());
				inst.Operands = { FOperand(EOperand::Indirect_Immediate_Word, ENumberDisplayMode::None, 0, nn), FOperand(rp[p]) };
				break;
			}
			case 5: /* LD HL/IX/IY,(nn)*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from memory locations %s and %s.", GetOperandName(rp[p]).c_str(), NumStr(nn), NumStr(uint16_t(nn + 1)));
				inst.Operands = { FOperand(rp[p]), FOperand(EOperand::Indirect_Immediate_Word, ENumberDisplayMode::None, 0, nn) };
				break;
			}
			case 6: /* LD (nn), A*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (%s) from A", NumStr(nn));
				inst.Operands = { FOperand(EOperand::Indirect_Immediate_Byte, ENumberDisplayMode::None, 0, nn), FOperand(EOperand::A) };
				break;
			}
			case 7: /* LD A, (nn)*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A from (%s)", NumStr(nn));
				inst.Operands = { FOperand(EOperand::A), FOperand(EOperand::Indirect_Immediate_Byte, ENumberDisplayMode::None, 0, nn) };
				break;
			}
			}
		}
		break;
		case 3: /* INC/DEC ss (register pair)*/
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s register pair %s", q == 0 ? "Increment" : "Decrement", GetOperandName(rp[p]).c_str());
			inst.Operands = { FOperand(EOperand::F), FOperand(rp[p]) };
			break;
		case 4: /* INC (HL)/(IX+d)/(IY+d) or r */
		{
			if (y == 6) /* (HL)/(IX+d)/(IY+d) */
			{
				if (prefix) // INC (IX+d)/(IY+d)
				{
					int8_t d = state.ReadByte(pc++);
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Increment %s.", GetOperandName(r[y], d).c_str());
					inst.Operands = { FOperand(r[y], ENumberDisplayMode::None, d) };
				}
				else // INC (HL)
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Increment (HL).");
					inst.Operands = { FOperand(EOperand::HL_Indirect) };
				}
			}
			else // INC r
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Increment %s.", GetOperandName(r[y]).c_str());
				inst.Operands = { FOperand(r[y]) };
			}
			inst.Operands.push_front(FOperand(EOperand::F));
			break;
		}
		case 5: /* DEC (HL)/(IX+d)/(IY+d) or r */
		{
			if (y == 6)
			{
				if (prefix) // DEC (IX+d)/(IY+d)
				{
					int8_t d = state.ReadByte(pc++);
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement %s.", GetOperandName(r[y], d).c_str());
					inst.Operands = { FOperand(r[y], ENumberDisplayMode::None, d) };
				}
				else // DEC (HL)
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement (HL).");
					inst.Operands = { FOperand(EOperand::HL_Indirect) };
				}
			}
			else // DEC r
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement %s.", GetOperandName(r[y]).c_str());
				inst.Operands = { FOperand(r[y]) };
			}
			inst.Operands.push_front(FOperand(EOperand::F));
			break;
		}
		case 6: /* LD s, n. Where s is (HL)/(IX+d)/(IY+d) or r*/
		{
			if (y == 6)
			{
				if (prefix) // LD (IX+d)/(IY+d), n
				{
					const int8_t d = state.ReadByte(pc++);
					const uint8_t n = state.ReadByte(pc);
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s with immediate data %s.", GetOperandName(r[y], d).c_str(), NumStr(n));
					inst.Operands = { FOperand(r[y], ENumberDisplayMode::None, d) };
				}
				else // LD (HL), n
				{
					const uint8_t n = state.ReadByte(pc);
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (HL) with immediate data %s.", NumStr(n));
					inst.Operands = { FOperand(EOperand::HL_Indirect) };
				}
			}
			else // LD r, n
			{
				const uint8_t n = state.ReadByte(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s with immediate data %s.", GetOperandName(r[y]).c_str(), NumStr(n));
				inst.Operands = { FOperand(r[y]) };
			}
			break;
		}
		case 7:
			if (y < 6) /* RLCA, RRCA, RLA, RRA, DAA, CPL, */
			{
				switch (y)
				{
				case 0: /* RLCA */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "A is rotated left 1 bit position. Bit 7 is moved to the C flag and also to bit 0.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate accumulator left with branch carry");
					break;
				case 1: /* RRCA */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "A is rotated right 1 bit position. Bit 0 is moved to the C flag and also to bit 7.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate accumulator right with branch carry");
					break;
				case 2: /* RLA */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "A is rotated left 1 bit position. The C flag is moved to bit 0 and bit 7 is moved to the C flag.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate accumulator left through carry flag");
					break;
				case 3: /* RRA */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "A is shifted right 1 bit position. The C flag is moved to bit 7 and bit 0 is moved to the C flag.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate accumulator right through carry");
					break;
				case 4: /* DAA */
					// todo: display table?
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Conditionally adjust A for BCD arithmetic operations.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Decimal adjust accumulator");
					break;
				case 5: /* CPL */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "A is inverted (one's complement). All bits in A are flipped.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Complement accumulator");
					break;
				}
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A, ENumberDisplayMode::Binary) };
			}
			else /* SCF, CCF*/
			{
				if (y == 6) /* SCF */
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Set C flag.");
				}
				else if (y == 7) /* CCF */
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Flip C flag. C = !C");
				}
				inst.Operands = { FOperand(EOperand::F) };
			}
			break;
		}
	}
	else
	{
		switch (z)
		{
		case 0: /* RET NZ/Z/NC/C/PO/PE/P/M*/
			switch (y)
			{
			case 0: /* NZ */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if Z flag is not set.");
				break;
			case 1: /* Z */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if Z flag is set.");
				break;
			case 2: /* NC */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if C flag is not set.");
				break;
			case 3: /* C */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if C flag is set.");
				break;
			case 4: /* PO */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if P flag is not set (parity odd).");
				break;
			case 5: /* PE */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if P flag is set (parity even).");
				break;
			case 6: /* P */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if S flag is set (sign positive).");
				break;
			case 7: /* M */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Return from subroutine if S flag is not set (sign negative).");
				break;
			}

			inst.Operands = { FOperand(EOperand::F) };
			break;
		case 1:
			if (q == 0) /* POP qq (register pair)*/
			{
				std::string strReg = GetOperandName(rp2[p]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(SP) is loaded into the low byte of %s. SP is incremented and (SP) is loaded into the high byte of %s and then SP is incremented again.", GetOperandName(rp2[p]).c_str(), GetOperandName(rp2[p]).c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Pop %s from stack.", strReg.c_str());
				inst.Operands = { FOperand(rp2[p]), FOperand(EOperand::SP_Indirect) };
			}
			else
			{
				switch (p)
				{
				case 0: /* RET*/
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "PC is popped from the stack. Normally used to return from a subroutine entered by a CALL instruction.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Return from subroutine");
					inst.Operands = { FOperand(EOperand::SP_Indirect) };
					break;
				}
				case 1: /* EXX */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "The contents of BC, DE and HL are exchanged with the contents of BC', DE' and HL'.");
					break;
				case 2: /* JP (HL)/(IX)/(IY)*/
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to (%s)", GetOperandName(rp[2]).c_str());
					inst.Operands = { FOperand(rp2[2]) };
					break;
				case 3: /* LD SP, HL/IX/IY*/
					// todo add HL IX and IY in here
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load SP from %s", GetOperandName(rp[2]).c_str());
					inst.Operands = { FOperand(EOperand::SP), FOperand(rp[2]) };
					break;
				}
			}
			break;
		case 2: /* JP cc, nn. Where cc is NZ/Z/NC/C/PO/PE/P/M*/
		{
			const char* pchnn = NumStr(state.ReadWord(pc));
			switch (y)
			{
			case 0: /* NZ */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if Z flag is not set.", pchnn);
				break;
			case 1: /* Z */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if Z flag is set.", pchnn);
				break;
			case 2: /* NC */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if C flag is not set.", pchnn);
				break;
			case 3: /* C */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if C flag is set.", pchnn);
				break;
			case 4: /* PO */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if P flag is not set (parity odd).", pchnn);
				break;
			case 5: /* PE */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if P flag is set (parity even).", pchnn);
				break;
			case 6: /* P */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if S flag is set (sign positive).", pchnn);
				break;
			case 7: /* M */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s if S flag is not set (sign negative).", pchnn);
				break;
			}

			inst.Operands = { FOperand(EOperand::F) };
			break;
		}
		case 3:
			switch (y)
			{
			case 0: /* JP nn*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to %s.", NumStr(state.ReadWord(pc)));
				break;
			case 2: /* OUT (n), A*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Contents of A is written to port (%s).", NumStr(state.ReadByte(pc)));
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Output A to port number specified in immediate data");
				inst.Operands = { FOperand(EOperand::A) };
				break;
			case 3: /* IN A, (n)*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (%s) is read and the result is loaded into A.", NumStr(state.ReadByte(pc)));
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Load A from port number specified in immediate data");
				inst.Operands = { FOperand(EOperand::A) };
				break;
			case 4: /* EX (SP), HL/IX/IY*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Exchange %s with top of stack.", GetOperandName(rp[2]).c_str());
				inst.Operands = { FOperand(rp[2]), FOperand(EOperand::SP_Indirect) };
				break;
			case 5: /* EX DE,HL*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Exchange the HL and DE registers");
				inst.Operands = { FOperand(EOperand::DE), FOperand(EOperand::HL) };
				break;
			case 6: /* DI*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Disable maskable interrupts.");
				break;
			case 7: /* EI*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Enable maskable interrupts.");
				break;
			case 1: /* CB prefix */
				DoCBPrefix(prefix, r, pc, state, inst);
				break;
			}
			break;
		case 4: /* CALL cc, nn. Where cc is NZ/Z/NC/C/PO/PE/P/M*/
		{
			const char* pchnn = NumStr(state.ReadWord(pc));
			switch (y)
			{
			case 0: /* NZ */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if Z flag is not set.", pchnn);
				break;
			case 1: /* Z */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if Z flag is set.", pchnn);
				break;
			case 2: /* NC */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if C flag is not set.", pchnn);
				break;
			case 3: /* C */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if C flag is set.", pchnn);
				break;
			case 4: /* PO */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if P flag is not set (parity odd).", pchnn);
				break;
			case 5: /* PE */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if P flag is set (parity even).", pchnn);
				break;
			case 6: /* P */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if S flag is set (sign positive).", pchnn);
				break;
			case 7: /* M */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Call subroutine at %s if S flag is not set (sign negative).", pchnn);
				break;
			}

			inst.Operands = { FOperand(EOperand::F) };
			break;
		}
		case 5:
			if (q == 0)
			{
				/* PUSH qq (register pair)*/
				std::string strReg = GetOperandName(rp2[p]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "SP is decremented and the high byte of %s is loaded into (SP). SP is decremented again and the low byte of %s is loaded into (SP).", strReg.c_str(), strReg.c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Push %s onto stack.", strReg.c_str());
				inst.Operands = { FOperand(EOperand::SP_Indirect), FOperand(rp2[p]) };
			}
			else
			{
				switch (p)
				{
				case 0: /* CALL nn*/
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "PC is pushed onto the stack. Execution continues from the address specified in the immediate data.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Call subroutine at location %s", NumStr(state.ReadWord(pc)));
					break;
				case 1: /* DBL PREFIX*/
					// not sure what opcode this is
					break;
				case 3: /* DBL PREFIX*/
					// not sure what opcode this is
					break;
				case 2: /* ED prefix */
					DoEDPrefix(prefix, r, rp, pc, state, inst);
					break;
				}
			}
			break;
		case 6: /* ADD A,n, ADC A,n, SUB n, SBC A,n, AND n, XOR n, OR n, CP n*/
		{
			uint8_t n = state.ReadByte(pc);
			const char* pchn = NumStr(n);
			switch (y)
			{
			case 0: // ADD A, n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Add A with immediate data %s. Result stored in A.", pchn);
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A) };
				break;
			case 1: // ADC A, n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Add immediate data %s and C flag to A. Result stored in A.", pchn);
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A) };
				break;
			case 2: // SUB A, n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract immediate data %s from A. Result stored in A.", pchn);
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A) };
				break;
			case 3: // SBC A, n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract immediate data %s and C flag from A. Result stored in A.", pchn);
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A) };
				break;
			case 4: // AND n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Logical AND A with immediate data %s (%s). Result stored in A.", pchn, NumStr(n, ENumberDisplayMode::Binary));
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A, ENumberDisplayMode::Binary) };
				break;
			case 5: // XOR n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Exclusive OR A with immediate data %s (%s). Result stored in A.", pchn, NumStr(n, ENumberDisplayMode::Binary));
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A, ENumberDisplayMode::Binary) };
				break;
			case 6: // OR n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Logical OR A with immediate data %s (%s). Result stored in A", pchn, NumStr(n, ENumberDisplayMode::Binary));
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A, ENumberDisplayMode::Binary) };
				break;
			case 7:
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract %s from A but discard result.\n\n"
					"Unsigned\nA == %s: Z flag is set.\nA != %s: Z flag is reset.\n"
					"A < %s:  C flag is set.\nA >= %s: C flag is reset.\n\n"
					"Signed\nA == %s: Z flag is set.\nA != %s: Z flag is reset.\n"
					"A < %s:  S and P/V are different.\nA >= %s: S and P/V are the same.\n",
					pchn, pchn, pchn, pchn, pchn, pchn, pchn, pchn, pchn);
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Compare A with immediate data %s", pchn);
				inst.Operands = { FOperand(EOperand::F), FOperand(EOperand::A) };
				break;
			}
			}
			break;
		}
		case 7: /* RST p */
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "The PC is pushed onto the stack and the program continues from address %s.", NumStr(uint16_t(y * 8)));
			break;
		}
	}

	inst.Description = g_TTZ80DescBuf;
	inst.Title = g_TTZ80TitleBuf;
}

std::string DisplacementStr(int8_t d)
{
	return std::string(d < 0 ? "-" : "+") + NumStr((uint8_t)(d < 0 ? -d : d));
}

// Get string of an operand.
std::string GetOperandName(EOperand operand, int8_t displacement /* = 0*/, uint16_t address /* = 0*/)
{
	switch (operand)
	{
		case EOperand::None:
			assert(0);
			return "?";
		case EOperand::A:
			return "A";
		case EOperand::B:
			return "B";
		case EOperand::C:
			return "C";
		case EOperand::D:
			return "D";
		case EOperand::E:
			return "E";
		case EOperand::H:
			return "H";
		case EOperand::L:
			return "L";
		case EOperand::I:
			return "I";
		case EOperand::R:
			return "R";
		case EOperand::IXL:
			return "IXL";
		case EOperand::IXH:
			return "IXH";
		case EOperand::IYL:
			return "IYL";
		case EOperand::IYH:
			return "IYH";
		case EOperand::F:
			return "F";
		case EOperand::AF:
			return "AF";
		case EOperand::AF_ALT:
			return "AF'";
		case EOperand::BC:
		case EOperand::BC_Indirect:
			return "BC";
		case EOperand::DE:
		case EOperand::DE_Indirect:
			return "DE";
		case EOperand::HL:
		case EOperand::HL_Indirect:
			return "HL";
		case EOperand::IX:
		case EOperand::IX_Indirect:
			return "IX";
		case EOperand::IY:
		case EOperand::IY_Indirect:
			return "IY";
		case EOperand::SP:
		case EOperand::SP_Indirect:
			return "SP";

		case EOperand::IX_Indirect_D:
			return std::string("IX") + DisplacementStr(displacement);
		case EOperand::IY_Indirect_D:
			return std::string("IY") + DisplacementStr(displacement);
		case EOperand::Indirect_Immediate_Word:
		case EOperand::Indirect_Immediate_Byte:
			// do nothing
			break;
	}
	
	return "";
}

bool GetOperandWordValue(FCodeAnalysisState& state, FOperand operand, uint16_t& value)
{
	z80_t* pCPU = (z80_t*)state.GetCPUInterface()->GetCPUEmulator();

	switch (operand.Type)
	{
	case EOperand::None:
	case EOperand::A:
	case EOperand::B:
	case EOperand::C:
	case EOperand::D:
	case EOperand::E:
	case EOperand::H:
	case EOperand::L:
	case EOperand::I:
	case EOperand::R:
	case EOperand::IXL:
	case EOperand::IXH:
	case EOperand::IYL:
	case EOperand::IYH:
	case EOperand::F:
		return false;
	case EOperand::AF:
		value = pCPU->af;
		break;
	case EOperand::AF_ALT:
		value = pCPU->af2;
		break;
	case EOperand::BC:
	case EOperand::BC_Indirect:
		value = pCPU->bc;
		break;
	case EOperand::DE:
	case EOperand::DE_Indirect:
		value = pCPU->de;
		break;
	case EOperand::HL:
	case EOperand::HL_Indirect:
		value = pCPU->hl;
		break;
	case EOperand::IX:
	case EOperand::IX_Indirect:
		value = pCPU->ix;
		break;
	case EOperand::IY:
	case EOperand::IY_Indirect:
		value = pCPU->iy;
		break;
	case EOperand::SP:
	case EOperand::SP_Indirect:
		value = pCPU->sp;
		break;
	case EOperand::IX_Indirect_D:
		value = pCPU->ix + operand.Displacement;
		break;
	case EOperand::IY_Indirect_D:
		value = pCPU->iy + operand.Displacement;
		break;
	case EOperand::Indirect_Immediate_Byte:
	case EOperand::Indirect_Immediate_Word:
		value = operand.Address;
		break;
	}

	return true;
}

std::string GetFlagsString(uint8_t flagsValue)
{
	char f_str[] = {
		'[',
		(flagsValue & Z80_SF) ? 'S' : '-',
		(flagsValue & Z80_ZF) ? 'Z' : '-',
		(flagsValue & Z80_YF) ? 'X' : '-',
		(flagsValue & Z80_HF) ? 'H' : '-',
		(flagsValue & Z80_XF) ? 'Y' : '-',
		(flagsValue & Z80_VF) ? 'V' : '-',
		(flagsValue & Z80_NF) ? 'N' : '-',
		(flagsValue & Z80_CF) ? 'C' : '-',
		']',
		0, };
	return ("Flags:" + std::string(f_str));
}

bool GetPreviousByteRegValue(EOperand op, uint8_t& value)
{
	const FZ80DisplayRegisters& oldRegs = GetStoredRegisters_Z80();
	switch (op)
	{
	case EOperand::None:
		assert(0);
		return false;

	case EOperand::A:
		value = oldRegs.A;
		break;
	case EOperand::B:
		value = oldRegs.B;
		break;
	case EOperand::C:
		value = oldRegs.C;
		break;
	case EOperand::D:
		value = oldRegs.D;
		break;
	case EOperand::E:
		value = oldRegs.E;
		break;
	case EOperand::H:
		value = oldRegs.H;
		break;
	case EOperand::L:
		value = oldRegs.L;
		break;
	case EOperand::I:
		value = oldRegs.I;
		break;
	case EOperand::R:
		value = oldRegs.R;
		break;
	case EOperand::IXL:
		value = (oldRegs.IX & 0xff);
		break;
	case EOperand::IXH:
		value = oldRegs.IX >> 8;
		break;
	case EOperand::IYL:
		value = (oldRegs.IY & 0xff);
		break;
	case EOperand::IYH:
		value = oldRegs.IY >> 8;
		break;
	case EOperand::F:
		value = oldRegs.F;
		break;

	case EOperand::AF:
	case EOperand::AF_ALT:
	case EOperand::BC:
	case EOperand::DE:
	case EOperand::HL:
	case EOperand::IX:
	case EOperand::IY:
	case EOperand::SP:
	case EOperand::BC_Indirect:
	case EOperand::DE_Indirect:
	case EOperand::HL_Indirect:
	case EOperand::IX_Indirect:
	case EOperand::IY_Indirect:
	case EOperand::SP_Indirect:
	case EOperand::IX_Indirect_D:
	case EOperand::IY_Indirect_D:
	case EOperand::Indirect_Immediate_Word:
	case EOperand::Indirect_Immediate_Byte:
		return false;
	}

	return true;
}

bool GetPreviousWordRegValue(EOperand op, uint16_t& value)
{
	const FZ80DisplayRegisters& oldRegs = GetStoredRegisters_Z80();
	switch (op)
	{
	case EOperand::None:
		assert(0);
		return false;

	case EOperand::AF:
		value = (oldRegs.A << 8) | oldRegs.F;
		break;
	case EOperand::AF_ALT:
		value = (oldRegs.A2 << 8) | oldRegs.F2;
		break;
	case EOperand::BC:
	case EOperand::BC_Indirect:
		value = oldRegs.BC;
		break;
	case EOperand::DE:
	case EOperand::DE_Indirect:
		value = oldRegs.DE;
		break;
	case EOperand::HL:
	case EOperand::HL_Indirect:
		value = oldRegs.HL;
		break;
	case EOperand::IX:
	case EOperand::IX_Indirect:
	case EOperand::IX_Indirect_D:
		value = oldRegs.IX;
		break;
	case EOperand::IY:
	case EOperand::IY_Indirect:
	case EOperand::IY_Indirect_D:
		value = oldRegs.IY;
		break;
	case EOperand::SP:
	case EOperand::SP_Indirect:
		value = oldRegs.SP;
		break;

	case EOperand::I:
	case EOperand::R:
	case EOperand::IXL:
	case EOperand::IXH:
	case EOperand::IYL:
	case EOperand::IYH:
	case EOperand::F:
	case EOperand::Indirect_Immediate_Word:
	case EOperand::Indirect_Immediate_Byte:
		return false;
	}

	return true;
}

void DrawByteOperand(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FOperand* pOperand)
{
	const ImVec4 regNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 regChangedCol(1.0f, 1.0f, 0.0f, 1.0f);

	std::string regName = GetOperandName(pOperand->Type);
	bool bGotValueOk = true;
	uint8_t regValue = 0;
	bool bValueChanged = false;

	bGotValueOk = state.Debugger.GetRegisterByteValue(regName.c_str(), regValue);
	uint8_t oldValue;
	if (GetPreviousByteRegValue(pOperand->Type, oldValue))
		bValueChanged = oldValue != regValue;

	if (bGotValueOk)
	{
		const ImVec4& col = bValueChanged ? regChangedCol : regNormalCol;
		if (pOperand->Type == EOperand::F)
		{
			ImGui::TextColored(col, "%s", GetFlagsString(regValue).c_str());
		}
		else
		{
			std::string altStr;
			if (pOperand->DisplayMode != ENumberDisplayMode::None && pOperand->DisplayMode != GetNumberDisplayMode())
				altStr = NumStr(regValue, pOperand->DisplayMode);

			ImGui::TextColored(col, "%s:%s %s", regName.c_str(), NumStr(regValue), altStr.c_str());
		}
	}
}

void DrawWordOperand(FCodeAnalysisState& state, FCodeAnalysisViewState& viewState, const FOperand& operand)
{
	const ImVec4 regNormalCol(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec4 regChangedCol(1.0f, 1.0f, 0.0f, 1.0f);

	static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	char tableName[16];
	snprintf(tableName, 16, "16BitOperand%d", operand.Type);
	ImGui::BeginTable(tableName, 1, tableFlags);
	ImGui::TableNextColumn();

	std::string regName = GetOperandName(operand.Type, operand.Displacement, operand.Address);
	uint16_t regValue = 0;
	bool bValueChanged = false;
	bool bGotValueOk = true;
	
	bGotValueOk = GetOperandWordValue(state, operand, regValue);

	uint16_t oldValue;
	if (GetPreviousWordRegValue(operand.Type, oldValue))
		bValueChanged = oldValue != regValue;

	if (bGotValueOk)
	{
		const bool bTreatAsAddress = operand.IsAddress() && operand.DisplayMode != ENumberDisplayMode::Decimal;

		if (!regName.empty())
			regName += ":";

		// Draw address label
		if (bTreatAsAddress)
		{
			ImGui::TextColored(bValueChanged ? regChangedCol : regNormalCol, "%s%s", regName.c_str(), NumStr(regValue));
			DrawAddressLabel(state, viewState, regValue);

			if (operand.Type == EOperand::HL_Indirect)
			{
				ImGui::SameLine();
				ImGui::Text(" = %s", NumStr(state.ReadByte(regValue)));
			}
		}
		else
		{
			// todo colorise this?
			const ENumberDisplayMode numMode = operand.DisplayMode == ENumberDisplayMode::None ? GetNumberDisplayMode() : operand.DisplayMode;
			ImGui::Text("%s%s", regName.c_str(), NumStr(regValue, numMode));
		}

		// Draw snippet
		if (bTreatAsAddress)
		{
			const FAddressRef regAddr = state.AddressRefFromPhysicalAddress(regValue);
			viewState.HighlightAddress = regAddr;
			ImGui::Spacing();
			DrawSnippetToolTip(state, viewState, regAddr, 5);
		}
	}
	ImGui::EndTable();
}

void DrawByteOperandList(FCodeAnalysisState& state, FCodeAnalysisViewState viewState, const TOperandList& operandList)
{
	static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame;

	if (ImGui::BeginTable("ByteOperands", (int)operandList.size(), tableFlags))
	{
		for (const Z80ToolTip::FOperand* pOperand : operandList)
		{
			ImGui::TableNextColumn();
			DrawByteOperand(state, viewState, pOperand);
		}
		ImGui::EndTable();
	}
}

void DrawWordOperandList(FCodeAnalysisState& state, FCodeAnalysisViewState viewState, const TOperandList& operandList)
{
	static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame;

	for (const FOperand* pOperand : operandList)
	{
		DrawWordOperand(state, viewState, *pOperand);
	}
}

} // namespace Z80ToolTip

void ShowCodeToolTipZ80(FCodeAnalysisState& state, uint16_t addr)
{
	using namespace Z80ToolTip;

	// Get register usage and try to auto generate a description for the instruction.
	FInstruction instrInfo;
	GetInstructionInfo(addr, state, instrInfo);

	if (instrInfo.Operands.empty() && instrInfo.Description.empty())
		return;

	ImGui::BeginTooltip();

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const float wrap_width = 30 * ImGui::GetFontSize();

	// Draw title (if there is one)
	if (!instrInfo.Title.empty())
	{
		ImVec2 itemRectMax = ImGui::GetItemRectSize();

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x - 20, pos.y - 4), ImVec2(pos.x + itemRectMax.x, pos.y + ImGui::GetTextLineHeightWithSpacing()), IM_COL32(64, 64, 64, 255));

		ImGui::Text("%s", instrInfo.Title.c_str());
		ImGui::Separator();
	}

	// Draw description
	if (!instrInfo.Description.empty())
	{
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
		ImGui::TextUnformatted(instrInfo.Description.c_str());
		ImGui::PopTextWrapPos();
	}

	z80_t* pCPU = (z80_t*)state.GetCPUInterface()->GetCPUEmulator();
	FCodeAnalysisViewState& viewState = state.GetFocussedViewState();

	
	static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame;

	if (instrInfo.Operands.size())
	{
		// Split operands into a list per type (byte & word). Flags get their own list.
		std::vector<TOperandList> operandLists;
		for (FOperand& operand : instrInfo.Operands)
		{
			if (operandLists.empty() || operandLists.back().back()->IsByteOperand() != operand.IsByteOperand() || (operandLists.back().back()->Type == EOperand::F))
				operandLists.push_back(TOperandList());

			if (operandLists.back().empty() || operandLists.back().back()->Type != operand.Type)
				operandLists.back().push_back(&operand);
		}

		for (const TOperandList& operandList : operandLists)
		{
			if (operandList.front()->IsByteOperand())
			{
				DrawByteOperandList(state, viewState, operandList);
			}
			else
			{
				DrawWordOperandList(state, viewState, operandList);
			}
		}
	}
	
	ImGui::EndTooltip();
}
