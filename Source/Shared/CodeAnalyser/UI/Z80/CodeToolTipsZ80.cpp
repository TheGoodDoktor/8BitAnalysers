#include "CodeToolTipsZ80.h"
#include "../CodeToolTips.h"
#include "../../CodeAnalyser.h"

#include <Util/Misc.h>
#include <chips/z80.h>

#include "imgui.h"

#include <map>

struct FToolTipReg
{
	FToolTipReg()
	{
	}
	FToolTipReg(int8_t displacement, ENumberDisplayMode displayMode = GetNumberDisplayMode())
		: Displacement(displacement),
		DisplayMode(displayMode) { }
	ENumberDisplayMode DisplayMode = ENumberDisplayMode::None;
	int8_t Displacement = 0;
};


struct FToolTipInstructionInfo
{
	uint32_t RegFlags = 0;
	std::string Title;
	std::string Description;
	std::map<uint32_t, FToolTipReg> RegInfoMap;
};

// register bits
namespace Z80Reg
{
	const uint32_t	A = 0x00000001;
	const uint32_t	F = 0x00000002;
	const uint32_t	B = 0x00000004;
	const uint32_t	C = 0x00000008;
	const uint32_t	D = 0x00000010;
	const uint32_t	E = 0x00000020;
	const uint32_t	H = 0x00000040;
	const uint32_t	L = 0x00000080;
	const uint32_t	I = 0x00000100;
	const uint32_t	R = 0x00000200;
	const uint32_t	IXL = 0x00000400;
	const uint32_t	IXH = 0x00000800;
	const uint32_t	IYL = 0x00001000;
	const uint32_t	IYH = 0x00002000;

	const uint32_t	SP = 0x00004000;
	const uint32_t	AF = 0x00008000;
	const uint32_t	BC = 0x00010000;
	const uint32_t	DE = 0x00020000;
	const uint32_t	HL = 0x00040000;
	const uint32_t	IX = 0x00080000;
	const uint32_t	IY = 0x00100000;

	const uint32_t	IX_Indirect_D = 0x00200000;
	const uint32_t	IY_Indirect_D = 0x00400000;

	const uint32_t	AF_ALT = 0x00800000;
	const uint32_t	SPARE_BIT_2 = 0x01000000;
	const uint32_t	SPARE_BIT_3 = 0x02000000;

	const uint32_t	BC_Indirect = 0x04000000;
	const uint32_t	DE_Indirect = 0x08000000;
	const uint32_t	HL_Indirect = 0x10000000;
	const uint32_t	IX_Indirect = 0x20000000;
	const uint32_t	IY_Indirect = 0x40000000;
	const uint32_t	SP_Indirect = 0x80000000;

	// todo: add PC?
}

static uint32_t g_TTZ80Reg[8] = { Z80Reg::B, Z80Reg::C, Z80Reg::D, Z80Reg::E, Z80Reg::H,   Z80Reg::L,   Z80Reg::HL_Indirect, Z80Reg::A };
static uint32_t g_TTZ80RegIX[8] = { Z80Reg::B, Z80Reg::C, Z80Reg::D, Z80Reg::E, Z80Reg::IXH, Z80Reg::IXL, Z80Reg::IX_Indirect_D, Z80Reg::A };
static uint32_t g_TTZ80RegIY[8] = { Z80Reg::B, Z80Reg::C, Z80Reg::D, Z80Reg::E, Z80Reg::IYH, Z80Reg::IYL, Z80Reg::IY_Indirect_D, Z80Reg::A };

static uint32_t g_TTZ80RegPairs[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::HL, Z80Reg::SP };
static uint32_t g_TTZ80RegPairsIX[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IX, Z80Reg::SP };
static uint32_t g_TTZ80RegPairsIY[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IY, Z80Reg::SP };

static uint32_t g_TTZ80RegPairs2[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::HL, Z80Reg::AF };
static uint32_t g_TTZ80RegPairs2IX[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IX, Z80Reg::AF };
static uint32_t g_TTZ80RegPairs2IY[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IY, Z80Reg::AF };

const int kTTZ80DescLen = 512;
const int kTTZ80TitleLen = 128;
static char g_TTZ80DescBuf[kTTZ80DescLen];
static char g_TTZ80TitleBuf[kTTZ80TitleLen];

std::string DisplacementStr(int8_t d)
{
	return std::string(d < 0 ? "-" : "+") + NumStr((uint8_t)(d < 0 ? -d : d));
}

// Get string of a single register.
std::string GetRegName(uint32_t flag, int8_t d = 0)
{
	if (flag & Z80Reg::A)
		return "A";
	if (flag & Z80Reg::F)
		return "F";
	if (flag & Z80Reg::B)
		return "B";
	if (flag & Z80Reg::C)
		return "C";
	if (flag & Z80Reg::D)
		return "D";
	if (flag & Z80Reg::E)
		return "E";
	if (flag & Z80Reg::H)
		return "H";
	if (flag & Z80Reg::L)
		return "L";
	if (flag & Z80Reg::I)
		return "I";
	if (flag & Z80Reg::R)
		return "R";
	if (flag & Z80Reg::SP)
		return "SP";
	if (flag & Z80Reg::AF)
		return "AF";
	if (flag & Z80Reg::AF_ALT)
		return "AF'";
	if (flag & Z80Reg::BC)
		return "BC";
	if (flag & Z80Reg::DE)
		return "DE";
	if (flag & Z80Reg::HL)
		return "HL";
	if (flag & Z80Reg::IX)
		return "IX";
	if (flag & Z80Reg::IY)
		return "IY";
	if (flag & Z80Reg::IXL)
		return "IXL";
	if (flag & Z80Reg::IYH)
		return "IXH";
	if (flag & Z80Reg::IXL)
		return "IYL";
	if (flag & Z80Reg::IYH)
		return "IYH";
	if (flag & Z80Reg::BC_Indirect)
		return "(BC)";
	if (flag & Z80Reg::DE_Indirect)
		return "(DE)";
	if (flag & Z80Reg::HL_Indirect)
		return "(HL)";
	if (flag & Z80Reg::IX_Indirect)
		return "(IX)";
	if (flag & Z80Reg::IY_Indirect)
		return "(IY)";
	if (flag & Z80Reg::SP_Indirect)
		return "(SP)";
	if (flag & Z80Reg::IX_Indirect_D)
		return std::string("(IX") + DisplacementStr(d) + ")";
	if (flag & Z80Reg::IY_Indirect_D)
		return std::string("(IY") + DisplacementStr(d) + ")";

	return "";
}


void GetTooltipRegStr(uint32_t regFlag, uint32_t regFlag2, uint16_t value, bool isByte, FToolTipInstructionInfo& inst, std::vector<std::string>& strings)
{
	g_TTZ80DescBuf[0] = 0;

	if (inst.RegFlags & regFlag || inst.RegFlags & regFlag2)
	{
		std::map<uint32_t, FToolTipReg>::iterator it = inst.RegInfoMap.find(regFlag);
		const char* numStr = isByte ? NumStr((uint8_t)value) : NumStr(value);
		std::string numStrAlt;
		if (it != inst.RegInfoMap.end())
		{
			if (it->second.DisplayMode != GetNumberDisplayMode())
			{
				//numStrAlt += " (";
				numStrAlt += ' ';
				numStrAlt += isByte ? NumStr((uint8_t)value, it->second.DisplayMode) : NumStr(value, it->second.DisplayMode);
				//numStrAlt += ')';
			}
		}
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s:%s%s ", GetRegName(regFlag).c_str(), numStr, numStrAlt.c_str());
		strings.push_back(g_TTZ80DescBuf);
	}
}

void GetTooltipRegStrByte(uint32_t regFlag, uint32_t regFlag2, uint16_t value, FToolTipInstructionInfo& inst, std::vector<std::string>& strings)
{
	GetTooltipRegStr(regFlag, regFlag2, value, true /* isByte */, inst, strings);
}

void GetTooltipRegStrWord(uint32_t regFlag, uint32_t regFlag2, uint16_t value, FToolTipInstructionInfo& inst, std::vector<std::string>& strings)
{
	GetTooltipRegStr(regFlag, regFlag2, value, false /* isByte */, inst, strings);
}

void GetTooltipRegStrIndirect(uint32_t regFlag, uint16_t addr, bool isByte, FToolTipInstructionInfo& inst, ICPUInterface* CPUIF, std::vector<std::string>& strings)
{
	g_TTZ80DescBuf[0] = 0;

	if (inst.RegFlags & regFlag)
	{
		std::map<uint32_t, FToolTipReg>::iterator it = inst.RegInfoMap.find(regFlag);
		const uint8_t d = it == inst.RegInfoMap.end() ? 0 : it->second.Displacement;
		uint16_t value = 0;
		const char* numStr = 0;
		if (isByte)
		{
			value = CPUIF->ReadByte(addr+d);
			numStr = NumStr((uint8_t)value);
		}
		else
		{
			value = CPUIF->ReadWord(addr+d);
			numStr = NumStr(value);
		}
		std::string numStrAlt;
		if (it != inst.RegInfoMap.end())
		{
			if (it->second.DisplayMode != GetNumberDisplayMode())
			{
				//numStrAlt += " (";
				numStrAlt += ' ';
				numStrAlt += isByte ? NumStr((uint8_t)value, it->second.DisplayMode) : NumStr(value, it->second.DisplayMode);
				//numStrAlt += ')';
			}
		}
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s:%s%s ", GetRegName(regFlag, d).c_str(), numStr, numStrAlt.c_str());
		strings.push_back(g_TTZ80DescBuf);
	}
}

void GetTooltipRegStrIndirectByte(uint32_t regFlag, uint16_t addr, FToolTipInstructionInfo& inst, ICPUInterface* CPUIF, std::vector<std::string>& strings)
{
	GetTooltipRegStrIndirect(regFlag, addr, true /* isByte */, inst, CPUIF, strings);
}

void GetTooltipRegStrIndirectWord(uint32_t regFlag, uint16_t addr, FToolTipInstructionInfo& inst, ICPUInterface* CPUIF, std::vector<std::string>& strings)
{
	GetTooltipRegStrIndirect(regFlag, addr, false /* isByte */, inst, CPUIF, strings);
}

void GenerateRegisterValueStrings(FToolTipInstructionInfo& inst, ICPUInterface* CPUIF, std::vector<std::string>& strings)
{
	z80_t* pCPU = (z80_t*)CPUIF->GetCPUEmulator();

	GetTooltipRegStrByte(Z80Reg::A,		0, z80_a(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::B,		0, z80_b(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::C,		0, z80_c(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::D,		0, z80_d(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::E,		0, z80_e(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::H,		0, z80_h(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::L,		0, z80_l(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::I,		0, z80_i(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::R,		0, z80_r(pCPU),			inst, strings);
	GetTooltipRegStrByte(Z80Reg::IXL,	0, z80_ix(pCPU) & 0xff, inst, strings);
	GetTooltipRegStrByte(Z80Reg::IXH,	0, z80_ix(pCPU) >> 8,	inst, strings);
	GetTooltipRegStrByte(Z80Reg::IYL,	0, z80_iy(pCPU) & 0xff, inst, strings);
	GetTooltipRegStrByte(Z80Reg::IYH,	0, z80_iy(pCPU) >> 8,	inst, strings);

	GetTooltipRegStrWord(Z80Reg::SP,		Z80Reg::SP_Indirect,	z80_sp(pCPU),  inst, strings);
	GetTooltipRegStrWord(Z80Reg::AF,		0,						z80_af(pCPU),  inst, strings);
	GetTooltipRegStrWord(Z80Reg::AF_ALT,	0,						z80_af_(pCPU), inst, strings);
	GetTooltipRegStrWord(Z80Reg::BC,		Z80Reg::BC_Indirect,	z80_bc(pCPU),  inst, strings);
	GetTooltipRegStrWord(Z80Reg::DE,		Z80Reg::DE_Indirect,	z80_de(pCPU),  inst, strings);
	GetTooltipRegStrWord(Z80Reg::HL,		Z80Reg::HL_Indirect,	z80_hl(pCPU),  inst, strings);
	GetTooltipRegStrWord(Z80Reg::IX,		Z80Reg::IX_Indirect,	z80_ix(pCPU),  inst, strings);
	GetTooltipRegStrWord(Z80Reg::IY,		Z80Reg::IY_Indirect,	z80_iy(pCPU),  inst, strings);

	GetTooltipRegStrIndirectByte(Z80Reg::BC_Indirect,		z80_bc(pCPU), inst, CPUIF, strings);
	GetTooltipRegStrIndirectByte(Z80Reg::DE_Indirect,		z80_de(pCPU), inst, CPUIF, strings);
	GetTooltipRegStrIndirectByte(Z80Reg::HL_Indirect,		z80_hl(pCPU), inst, CPUIF, strings);
	GetTooltipRegStrIndirectByte(Z80Reg::IX_Indirect,		z80_ix(pCPU), inst, CPUIF, strings);
	GetTooltipRegStrIndirectByte(Z80Reg::IY_Indirect,		z80_iy(pCPU), inst, CPUIF, strings);
	GetTooltipRegStrIndirectWord(Z80Reg::SP_Indirect,		z80_sp(pCPU), inst, CPUIF, strings);
	GetTooltipRegStrIndirectByte(Z80Reg::IX_Indirect_D,		z80_ix(pCPU), inst, CPUIF, strings);
	GetTooltipRegStrIndirectByte(Z80Reg::IY_Indirect_D,		z80_iy(pCPU), inst, CPUIF, strings);

	if (inst.RegFlags & Z80Reg::F)
	{
		//snprintf(tempStr, kTempStrLen, "F: %s ", NumStr(z80_f(pCPU)));
		//outString += std::string(tempStr);

		const uint8_t f = z80_f(pCPU);
		char f_str[] = {
			'[',
			(f & Z80_SF) ? 'S' : '-',
			(f & Z80_ZF) ? 'Z' : '-',
			(f & Z80_YF) ? 'X' : '-',
			(f & Z80_HF) ? 'H' : '-',
			(f & Z80_XF) ? 'Y' : '-',
			(f & Z80_VF) ? 'V' : '-',
			(f & Z80_NF) ? 'N' : '-',
			(f & Z80_CF) ? 'C' : '-',
			']',
			0, };
		strings.push_back("Flags:" + std::string(f_str));
	}
}

void Do8BitLoadBlock(uint8_t y, uint8_t z, uint8_t prefix, uint32_t* r, uint16_t pc, const FCodeAnalysisState& state, FToolTipInstructionInfo& inst)
{
	int8_t d = 0;
	if (prefix)
	{
		d = state.ReadByte(pc);
		inst.RegInfoMap[r[6]] = FToolTipReg(d);
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
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetRegName(r[6], d).c_str(), GetRegName(g_TTZ80Reg[z]).c_str());
				inst.RegFlags = g_TTZ80Reg[z] | r[6];
			}
			else
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetRegName(r[6], d).c_str(), GetRegName(r[z]).c_str());
				inst.RegFlags = r[z] | r[6];
			}
		}
	}
	else if (z == 6)
	{
		/* LD r,(HL); LD r,(IX+d); LD r,(IY+d) */
		if (prefix && ((y == 4) || (y == 5)))
		{
			/* special case LD H/L,(IX+d) (don't use IXL/IXH) */
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetRegName(g_TTZ80Reg[y]).c_str(), GetRegName(r[6], d).c_str());
			inst.RegFlags = g_TTZ80Reg[y] | r[6];
		}
		else
		{
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetRegName(r[y]).c_str(), GetRegName(r[6], d).c_str());
			inst.RegFlags = r[y] | r[6];
		}
	}
	else
	{
		/* regular LD r,s */
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from %s.", GetRegName(r[y]).c_str(), GetRegName(r[z]).c_str());
		inst.RegFlags = g_TTZ80Reg[y] | g_TTZ80Reg[z];
	}
}

void Do8BitALUBlock(uint8_t y, uint8_t z, uint8_t prefix, uint32_t* r, uint16_t pc, const FCodeAnalysisState& state, FToolTipInstructionInfo& inst)
{
	int8_t d = 0;
	if (prefix)
	{
		d = state.ReadByte(pc);
	}

	const std::string regName = GetRegName(r[z], d);
	bool bBinary = false;

	switch (y)
	{
	case 0: // ADD A, r 
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Add %s to A. Result stored in A.", regName.c_str());
		break;
	case 1: // ADC A, s 
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Add %s and C flag to A. Result stored in A.", regName.c_str());
		inst.RegFlags |= Z80Reg::F;
		break;
	case 2: // SUB s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract %s from A. Result stored in A.", regName.c_str());
		break;
	case 3: // SBC A, s
		snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract %s and C flag from A. Result stored in A.", regName.c_str());
		inst.RegFlags |= Z80Reg::F;
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
		inst.RegFlags |= Z80Reg::F;
		break;
	}

	inst.RegFlags |= r[z] | Z80Reg::A;

	if (prefix || bBinary)
	{
		ENumberDisplayMode numMode = bBinary ? ENumberDisplayMode::Binary : GetNumberDisplayMode();
		inst.RegInfoMap[r[z]] = FToolTipReg(d, numMode);
		inst.RegInfoMap[Z80Reg::A] = FToolTipReg(d, numMode);
	}
}

void DoCBPrefix(uint8_t prefix, uint32_t* r, uint16_t pc, const FCodeAnalysisState& state, FToolTipInstructionInfo& inst)
{
	int8_t d = 0;
	uint8_t op = 0;

	if (prefix)
		d = state.ReadByte(pc++);
	op = state.ReadByte(pc++);

	const uint8_t x = (op >> 6) & 3;
	const uint8_t y = (op >> 3) & 7;
	const uint8_t z = op & 7;

	const std::string regName = GetRegName(r[z], d);

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
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Aithmetic shift right %s", regName.c_str());
			break;
		case 6: // SLL - Undocumented
			//snprintf(tempStr, kTempStrLen, "SLL %s", regName.c_str());
			break;
		case 7: // SRL s
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Rotate %s right one bit. Bit 0 is copied to the C flag and bit 7 is reset.", regName.c_str());
			snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Logical shift right %s", regName.c_str());
			break;
		}

		inst.RegFlags = r[z];
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
	inst.RegFlags = r[z];
	inst.RegInfoMap[r[z]] = FToolTipReg(d, ENumberDisplayMode::Binary);
}

void DoEDPrefix(uint8_t prefix, uint32_t* r, uint32_t* rp, uint16_t pc, const FCodeAnalysisState& state, FToolTipInstructionInfo& inst)
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
				inst.RegFlags = Z80Reg::BC | Z80Reg::HL | Z80Reg::DE | Z80Reg::DE_Indirect | Z80Reg::HL_Indirect;
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
				inst.RegFlags = Z80Reg::A | Z80Reg::BC | Z80Reg::HL | Z80Reg::HL_Indirect;
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
				inst.RegFlags = Z80Reg::B | Z80Reg::C | Z80Reg::HL | Z80Reg::HL_Indirect;
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

				inst.RegFlags = Z80Reg::B | Z80Reg::C | Z80Reg::HL | Z80Reg::HL_Indirect;
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
				const std::string strReg = GetRegName(r[y]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (C) is read and the result is loaded into %s.", strReg.c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Load %s from port (C)", strReg.c_str());
				inst.RegFlags = r[y] | Z80Reg::C;
			}
			break;
		case 1: /* OUT (C), r*/
			if (y != 6)
			{
				const std::string strReg = GetRegName(r[y]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "The contents of %s is written to port (C).", strReg.c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Output %s to port (C)", strReg.c_str());
				inst.RegFlags = r[y] | Z80Reg::C;
			}
			else // OUT (C), 0 [Undocumented]
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "0 is written to port (C).");
				inst.RegFlags = Z80Reg::C;
			}
			break;
		case 2: /* SBC HL, ss. ADC HL, ss. Where ss is BC/DE/HL/SP*/
			if (q == 0) // SBC HL, ss 
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s and C flag are subtracted from HL. Result stored in HL.", GetRegName(rp[p]).c_str());
			}
			else // ADC HL, ss
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s and C flag are added to HL. Result stored in HL.", GetRegName(rp[p]).c_str());
			}
			inst.RegFlags = Z80Reg::HL | rp[p] | Z80Reg::F;
			break;
		case 3: /* LD (nn), dd. LD dd, (nn). */
		{
			const uint16_t nn = state.ReadWord(pc);
			if (q == 0) // LD (nn), dd
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load memory locations (%s) and (%s) from %s.", NumStr(nn), NumStr(uint16_t(nn + 1)), GetRegName(rp[p]).c_str());
			}
			else // LD dd, (nn)
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from memory locations (%s) and (%s).", GetRegName(rp[p]).c_str(), NumStr(nn), NumStr(uint16_t(nn + 1)));
			}
			inst.RegFlags = rp[p];
			break;
		}
		case 4: /* NEG */
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Negate A (two's complement). Same result as subtracting A from 0.");
			inst.RegFlags = Z80Reg::A;
			inst.RegInfoMap[Z80Reg::A] = FToolTipReg(0, ENumberDisplayMode::Binary);
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
			inst.RegFlags = Z80Reg::SP_Indirect;
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
				break;
			case 1: /* LD R,A */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load Memory Refresh register R from A.");
				break;
			case 2: /* LD A,I */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A from Interrupt Vector Register I.");
				break;
			case 3: /* LD A,R */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A from Memory Refresh regiser R.");
				break;
			case 4: /* RRD */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Upper nibble of (HL) moved to lower nibble of (HL). Lower nibble of (HL) moved to lower nibble of A. Lower nibble of A moved to upper nibble of (HL). All operations occur simultaneously.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate right decimal");
				break;
			case 5: /* RLD */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Lower nibble of (HL) moved to lower nibble of (HL). Upper nibble of (HL) moved to lower nibble of A. Lower nibble of A moved to (HL). All operations occur simultaneously.");
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Rotate left decimal");
				break;
			case 6:
				// NOP (ED)
				break;
			case 7:
				// NOP (ED)
				break;
			}
			if (y == 0 || y == 2)
				inst.RegFlags = Z80Reg::A | Z80Reg::I;
			else
				if (y == 1 || y == 3)
					inst.RegFlags = Z80Reg::A | Z80Reg::R;
				else
					inst.RegFlags = Z80Reg::A | Z80Reg::HL_Indirect;
			break;
		}
	}
}

// Reference:
// http://www.z80.info/decoding.htm
void GetToolTipInfoFromOpcodeZ80(uint16_t pc, const FCodeAnalysisState& state, FToolTipInstructionInfo& inst)
{
	uint8_t op = 0, prefix = 0;
	uint32_t* r = g_TTZ80Reg;
	uint32_t* rp = g_TTZ80RegPairs;
	uint32_t* rp2 = g_TTZ80RegPairs2;
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
			r = g_TTZ80RegIX;
			rp = g_TTZ80RegPairsIX;
			rp2 = g_TTZ80RegPairs2IX;
		}
		else if (prefix == 0xFD)
		{
			r = g_TTZ80RegIY;
			rp = g_TTZ80RegPairsIY;
			rp2 = g_TTZ80RegPairs2IY;
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
				inst.RegFlags = Z80Reg::AF | Z80Reg::AF_ALT;
				break;
			case 2: /* DJNZ*/
			{
				const int8_t offset = state.ReadByte(pc);
				const uint16_t addr = pc + offset + 1;
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement B and jump %s to %s on no zero.", offset < 0 ? "back" : "forward", NumStr(addr));
				inst.RegFlags = Z80Reg::B | Z80Reg::F;
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
				inst.RegFlags = Z80Reg::F;
				break;
			}
			}
			break;
		case 1:
			if (q == 0) /* LD dd,nn*/
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s with immediate data %s.", GetRegName(rp[p]).c_str(), NumStr(state.ReadWord(pc)));
				inst.RegFlags = rp[p];
			}
			else /* ADD HL, ss. ADD IX, pp. ADD IY, rr. */
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "ADD %s to %s and store result in %s.", GetRegName(rp[2]).c_str(), GetRegName(rp[p]).c_str(), GetRegName(rp[2]).c_str());
				inst.RegFlags = rp[2] | rp[p];
			}
			break;
		case 2:
		{
			switch (y)
			{
			case 0: /* LD (BC),A */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (BC) with A.");
				inst.RegFlags = Z80Reg::A | Z80Reg::BC_Indirect;
				break;
			case 1: /* LD A,(BC) */
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A with (BC).");
				inst.RegFlags = Z80Reg::A | Z80Reg::BC_Indirect;
				break;
			case 2: /* LD (DE),A*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (DE) with A.");
				inst.RegFlags = Z80Reg::A | Z80Reg::DE_Indirect;
				break;
			case 3: /* LD A,(DE)*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A with (DE).");
				inst.RegFlags = Z80Reg::A | Z80Reg::DE_Indirect;
				break;
			case 4: /* LD (nn), HL/IX/IY*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load the memory locations %s and %s from %s.", NumStr(nn), NumStr(uint16_t(nn + 1)), GetRegName(rp[p]).c_str());
				inst.RegFlags = rp[p];
				break;
			}
			case 5: /* LD HL/IX/IY,(nn)*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s from memory locations %s and %s.", GetRegName(rp[p]).c_str(), NumStr(nn), NumStr(uint16_t(nn + 1)));
				inst.RegFlags = rp[p];
				break;
			}
			case 6: /* LD (nn), A*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (%s) from A", NumStr(nn));
				inst.RegFlags = Z80Reg::A;
				break;
			}
			case 7: /* LD A, (nn)*/
			{
				const uint16_t nn = state.ReadWord(pc);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load A from (%s)", NumStr(nn));
				inst.RegFlags = Z80Reg::A;
				break;
			}
			}
		}
		break;
		case 3: /* INC/DEC ss (register pair)*/
			snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "%s register pair %s", q == 0 ? "Increment" : "Decrement", GetRegName(rp[p]).c_str());
			inst.RegFlags = rp[p];
			break;
		case 4: /* INC (HL)/(IX+d)/(IY+d) or r */
		{
			if (y == 6) /* (HL)/(IX+d)/(IY+d) */
			{
				if (prefix) // INC (IX+d)/(IY+d)
				{
					int8_t d = state.ReadByte(pc++);
					inst.RegInfoMap[r[y]] = FToolTipReg(d);
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Increment %s.", GetRegName(r[y], d).c_str());
				}
				else // INC (HL)
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Increment (HL).");
				}
			}
			else // INC r
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Increment %s.", GetRegName(r[y]).c_str());
			}
			inst.RegFlags = r[y];
			break;
		}
		case 5: /* DEC (HL)/(IX+d)/(IY+d) or r */
		{
			if (y == 6)
			{
				if (prefix) // DEC (IX+d)/(IY+d)
				{
					int8_t d = state.ReadByte(pc++);
					inst.RegInfoMap[r[y]] = FToolTipReg(d);
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement %s.", GetRegName(r[y], d).c_str());
				}
				else // DEC (HL)
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement (HL).");

				}
			}
			else // DEC r
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Decrement %s.", GetRegName(r[y]).c_str());
			}
			inst.RegFlags |= r[y];
			break;
		}
		case 6: /* LD s, n. Where s is (HL)/(IX+d)/(IY+d) or r*/
			if (y == 6)
			{
				if (prefix) // LD (IX+d)/(IY+d), n
				{
					int8_t d = state.ReadByte(pc++);
					inst.RegInfoMap[r[y]] = FToolTipReg(d);
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s with immediate data %s.", GetRegName(r[y], d).c_str(), NumStr(state.ReadByte(pc + 1)));
				}
				else // LD (HL), n
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load (HL) with immediate data %s.", NumStr(state.ReadByte(pc)));
				}
			}
			else // LD r, n
			{
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load %s with immediate data %s.", GetRegName(r[y]).c_str(), NumStr(state.ReadByte(pc)));
			}
			inst.RegFlags |= r[y];
			break;
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
				inst.RegFlags = Z80Reg::A;
				inst.RegInfoMap[Z80Reg::A] = FToolTipReg(0, ENumberDisplayMode::Binary);
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
				inst.RegFlags = Z80Reg::F;
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

			inst.RegFlags = Z80Reg::F | Z80Reg::SP_Indirect;
			break;
		case 1:
			if (q == 0) /* POP qq (register pair)*/
			{
				std::string strReg = GetRegName(rp2[p]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "(SP) is loaded into the low byte of %s. SP is incremented and (SP) is loaded into the high byte of %s and then SP is incremented again.", GetRegName(rp2[p]).c_str(), GetRegName(rp2[p]).c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Pop %s from stack.", strReg.c_str());
				inst.RegFlags = rp2[p] | Z80Reg::SP_Indirect;
			}
			else
			{
				switch (p)
				{
				case 0: /* RET*/
				{
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "PC is popped from the stack. Normally used to return from a subroutine entered by a CALL instruction.");
					snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Return from subroutine");
					inst.RegFlags = Z80Reg::SP_Indirect;
					break;
				}
				case 1: /* EXX */
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "The contents of BC, DE and HL are exchanged with the contents of BC', DE' and HL'.");
					break;
				case 2: /* JP (HL)/(IX)/(IY)*/
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Jump to (%s)", GetRegName(rp[2]).c_str());
					inst.RegFlags = rp[2];
					break;
				case 3: /* LD SP, HL/IX/IY*/
					snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Load SP from %s", GetRegName(rp[2]).c_str());
					inst.RegFlags = rp[p];
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

			inst.RegFlags = Z80Reg::F;
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
				inst.RegFlags = Z80Reg::A;
				break;
			case 3: /* IN A, (n)*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Port (%s) is read and the result is loaded into A.", NumStr(state.ReadByte(pc)));
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Load A from port number specified in immediate data");
				inst.RegFlags = Z80Reg::A;
				break;
			case 4: /* EX (SP), HL/IX/IY*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Exchange %s with top of stack.", GetRegName(rp[2]).c_str());
				inst.RegFlags = rp[2] | Z80Reg::SP_Indirect;
				break;
			case 5: /* EX DE,HL*/
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Exchange the HL and DE registers");
				inst.RegFlags = Z80Reg::DE | Z80Reg::HL;
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

			inst.RegFlags = Z80Reg::F;
			break;
		}
		case 5:
			if (q == 0)
			{
				/* PUSH qq (register pair)*/
				std::string strReg = GetRegName(rp2[p]);
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "SP is decremented and the high byte of %s is loaded into (SP). SP is decremented again and the low byte of %s is loaded into (SP).", strReg.c_str(), strReg.c_str());
				snprintf(g_TTZ80TitleBuf, kTTZ80TitleLen, "Push %s onto stack.", strReg.c_str());
				inst.RegFlags = rp2[p] | Z80Reg::SP_Indirect;
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
				break;
			case 1: // ADC A, n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Add immediate data %s and C flag to A. Result stored in A.", pchn);
				inst.RegFlags |= Z80Reg::F;
				break;
			case 2: // SUB A, n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract immediate data %s from A. Result stored in A.", pchn);
				break;
			case 3: // SBC A, n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Subtract immediate data %s and C flag from A. Result stored in A.", pchn);
				inst.RegFlags |= Z80Reg::F;
				break;
			case 4: // AND n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Logical AND A with immediate data %s (%s). Result stored in A.", pchn, NumStr(n, ENumberDisplayMode::Binary));
				break;
			case 5: // XOR n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Exclusive OR A with immediate data %s (%s). Result stored in A.", pchn, NumStr(n, ENumberDisplayMode::Binary));
				break;
			case 6: // OR n
				snprintf(g_TTZ80DescBuf, kTTZ80DescLen, "Logical OR A with immediate data %s (%s). Result stored in A", pchn, NumStr(n, ENumberDisplayMode::Binary));
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
				inst.RegFlags |= Z80Reg::F;
				break;
			}
			}
			inst.RegFlags |= Z80Reg::A;
			if (y >= 4 && y <= 6)
			{
				inst.RegInfoMap[Z80Reg::A] = FToolTipReg(0, ENumberDisplayMode::Binary);
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

void ShowCodeToolTipZ80(FCodeAnalysisState& state, uint16_t addr)
{
	// Get flags for register usage and try to auto generate a description for the instruction.
	FToolTipInstructionInfo instrInfo;
	GetToolTipInfoFromOpcodeZ80(addr, state, instrInfo);

	std::vector<std::string> regStrs;
	GenerateRegisterValueStrings(instrInfo, state.CPUInterface, regStrs);

	if (!instrInfo.RegFlags && instrInfo.Description.empty())
		return;

	ImGui::BeginTooltip();

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const float wrap_width = 400.0f;
	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);

	// Draw title (if there is one)
	if (!instrInfo.Title.empty())
	{
		ImVec2 itemRectMax = ImGui::GetItemRectSize();

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x - 20, pos.y - 4), ImVec2(pos.x + itemRectMax.x, pos.y + ImGui::GetTextLineHeightWithSpacing()), IM_COL32(64, 64, 64, 255));

		ImGui::Text(instrInfo.Title.c_str());
		ImGui::Separator();
	}

	// Draw description
	if (!instrInfo.Description.empty())
	{
		ImGui::TextUnformatted(instrInfo.Description.c_str());
	}

	// Draw register values (if there are any)
	if (!regStrs.empty())
	{
		ImGui::Separator();

		static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("ToolTipRegisters", (int)regStrs.size(), flags))
		{
			for (const std::string& str : regStrs)
			{
				ImGui::TableNextColumn(); 
				ImGui::TextUnformatted(str.c_str());
			}
			ImGui::EndTable();
		}
		
		ImGui::PopTextWrapPos();
	}
	ImGui::EndTooltip();
}

