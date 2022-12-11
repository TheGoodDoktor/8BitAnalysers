#include "CodeToolTipsZ80.h"
#include "../CodeToolTips.h"
#include "../../CodeAnalyser.h"

#include <Util/Misc.h>
#include <chips/z80.h>

#include "imgui.h"

#include <map>

// register bits
namespace Z80Reg
{
	const uint32_t	A	= 0x00000001;
	const uint32_t	F	= 0x00000002;
	const uint32_t	B	= 0x00000004;
	const uint32_t	C	= 0x00000008;
	const uint32_t	D	= 0x00000010;
	const uint32_t	E	= 0x00000020;
	const uint32_t	H	= 0x00000040;
	const uint32_t	L	= 0x00000080;
	const uint32_t	I	= 0x00000100;
	const uint32_t	R	= 0x00000200;
	const uint32_t	IXL = 0x00000400;
	const uint32_t	IXH = 0x00000800;
	const uint32_t	IYL = 0x00001000;
	const uint32_t	IYH = 0x00002000;

	const uint32_t	SP	= 0x00004000;
	const uint32_t	AF	= 0x00008000;
	const uint32_t	BC	= 0x00010000;
	const uint32_t	DE	= 0x00020000;
	const uint32_t	HL	= 0x00040000;
	const uint32_t	IX	= 0x00080000;
	const uint32_t	IY	= 0x00100000;

	const uint32_t	BC_Indirect = 0x04000000;
	const uint32_t	DE_Indirect = 0x08000000;
	const uint32_t	HL_Indirect = 0x10000000;
	const uint32_t	IX_Indirect = 0x20000000;
	const uint32_t	IY_Indirect = 0x40000000;
	const uint32_t	SP_Indirect = 0x80000000;

	// todo: add PC and AF'?
}

std::string GetRegName(uint32_t flag)
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
	return "";
}

std::string GenerateRegisterValueString(uint32_t Regs, ICPUInterface* CPUIF)
{
	std::string outString;
	char tempStr[12];

	z80_t* pCPU = (z80_t *)CPUIF->GetCPUEmulator();

	// I was hoping there'd be a better way
	if (Regs & Z80Reg::A)
	{
		sprintf_s(tempStr, "A = %s ", NumStr(z80_a(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::B)
	{
		sprintf_s(tempStr, "B = %s ", NumStr(z80_b(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::C)
	{
		sprintf_s(tempStr, "C = %s ", NumStr(z80_c(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::D)
	{
		sprintf_s(tempStr, "D = %s ", NumStr(z80_d(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::E)
	{
		sprintf_s(tempStr, "E = %s ", NumStr(z80_e(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::H)
	{
		sprintf_s(tempStr, "H = %s ", NumStr(z80_h(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::L)
	{
		sprintf_s(tempStr, "L = %s ", NumStr(z80_l(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::I)
	{
		sprintf_s(tempStr, "I = %s ", NumStr(z80_i(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::R)
	{
		sprintf_s(tempStr, "R = %s ", NumStr(z80_r(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IXL)
	{
		const uint8_t ixl = static_cast<uint8_t>(z80_ix(pCPU) & 0xff);
		sprintf_s(tempStr, "IXL = %s ", NumStr(ixl));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IXH)
	{
		const uint8_t ixh = static_cast<uint8_t>(z80_ix(pCPU) >> 8);
		sprintf_s(tempStr, "IXH = %s ", NumStr(ixh));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IYL)
	{
		const uint8_t iyl = static_cast<uint8_t>(z80_iy(pCPU) & 0xff);
		sprintf_s(tempStr, "IYL = %s ", NumStr(iyl));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IYH)
	{
		const uint8_t iyh = static_cast<uint8_t>(z80_iy(pCPU) >> 8);
		sprintf_s(tempStr, "IYH = %s ", NumStr(iyh));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::SP || Regs & Z80Reg::SP_Indirect)
	{
		sprintf_s(tempStr, "SP = %s ", NumStr(z80_sp(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::AF)
	{
		sprintf_s(tempStr, "AF = %s ", NumStr(z80_af(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::BC || Regs & Z80Reg::BC_Indirect)
	{
		sprintf_s(tempStr, "BC = %s ", NumStr(z80_bc(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::DE || Regs & Z80Reg::DE_Indirect)
	{
		sprintf_s(tempStr, "DE = %s ", NumStr(z80_de(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::HL || Regs & Z80Reg::HL_Indirect)
	{
		sprintf_s(tempStr, "HL = %s ", NumStr(z80_hl(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IX || Regs & Z80Reg::IX_Indirect)
	{
		sprintf_s(tempStr, "IX = %s ", NumStr(z80_ix(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IY || Regs & Z80Reg::IY_Indirect)
	{
		sprintf_s(tempStr, "IY = %s ", NumStr(z80_iy(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::BC_Indirect)
	{
		const uint8_t val = CPUIF->ReadByte(z80_bc(pCPU));
		sprintf_s(tempStr, "(BC) = %s ", NumStr(val));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::DE_Indirect)
	{
		const uint8_t val = CPUIF->ReadByte(z80_de(pCPU));
		sprintf_s(tempStr, "(DE) = %s ", NumStr(val));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::HL_Indirect)
	{
		const uint8_t val = CPUIF->ReadByte(z80_hl(pCPU));
		sprintf_s(tempStr, "(HL) = %s ", NumStr(val));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IX_Indirect)
	{
		const uint8_t val = CPUIF->ReadByte(z80_ix(pCPU));
		sprintf_s(tempStr, "(IX) = %s ", NumStr(val));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IY_Indirect)
	{
		const uint8_t val = CPUIF->ReadByte(z80_iy(pCPU));
		sprintf_s(tempStr, "(IY) = %s ", NumStr(val));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::SP_Indirect)
	{
		const uint8_t val = CPUIF->ReadByte(z80_sp(pCPU));
		sprintf_s(tempStr, "(SP) = %s ", NumStr(val));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::F)
	{
		//sprintf_s(tempStr, "F = %s ", NumStr(z80_f(pCPU)));
		//outString += std::string(tempStr);

		const uint8_t f = z80_f(pCPU);
		char f_str[] = {
			'[',
			(f & Z80_SF) ? 'S':'-',
			(f & Z80_ZF) ? 'Z':'-',
			(f & Z80_YF) ? 'X':'-',
			(f & Z80_HF) ? 'H':'-',
			(f & Z80_XF) ? 'Y':'-',
			(f & Z80_VF) ? 'V':'-',
			(f & Z80_NF) ? 'N':'-',
			(f & Z80_CF) ? 'C':'-',
			']',
			0, };
		outString += "Flags = ";
		outString += std::string(f_str);
	}

	return outString;
}

struct FAutoGenInstructionInfo
{
	uint32_t RegFlags = 0;
	std::string Description;
};

static uint32_t gReg[8] =   { Z80Reg::B, Z80Reg::C, Z80Reg::D, Z80Reg::E, Z80Reg::H,   Z80Reg::L,   Z80Reg::HL_Indirect, Z80Reg::A };
static uint32_t gRegIX[8] = { Z80Reg::B, Z80Reg::C, Z80Reg::D, Z80Reg::E, Z80Reg::IXH, Z80Reg::IXL, Z80Reg::IX_Indirect, Z80Reg::A };
static uint32_t gRegIY[8] = { Z80Reg::B, Z80Reg::C, Z80Reg::D, Z80Reg::E, Z80Reg::IYH, Z80Reg::IYL, Z80Reg::IY_Indirect, Z80Reg::A };

static uint32_t gRegPairs[4] =	 { Z80Reg::BC, Z80Reg::DE, Z80Reg::HL, Z80Reg::SP};
static uint32_t gRegPairsIX[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IX, Z80Reg::SP};
static uint32_t gRegPairsIY[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IY, Z80Reg::SP};

static uint32_t gRegPairs2[4] =	  { Z80Reg::BC, Z80Reg::DE, Z80Reg::HL, Z80Reg::AF};
static uint32_t gRegPairs2IX[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IX, Z80Reg::AF};
static uint32_t gRegPairs2IY[4] = { Z80Reg::BC, Z80Reg::DE, Z80Reg::IY, Z80Reg::AF};

void GetFlagsAndGenerateDescriptionFromOpcode(uint16_t pc, ICPUInterface* CPUIF, FAutoGenInstructionInfo& inst) 
{
    uint8_t op = 0, pre = 0, u8 = 0;
    int8_t d = 0;
    uint16_t u16 = 0;
	uint32_t *r = gReg;
    uint32_t *rp = gRegPairs;
    uint32_t *rp2 = gRegPairs2;

	static char tempStr[512] = {0};

    /* fetch the first instruction byte */

	op = CPUIF->ReadByte(pc++);

    /* prefixed op? */
    if ((0xFD == op) || (0xDD == op)) 
	{
        pre = op;
		op = CPUIF->ReadByte(pc++);

        if (op == 0xED) 
		{
            pre = 0; /* an ED following a prefix cancels the prefix */
        }
        /* if prefixed op, use register tables that replace HL with IX/IY */
        if (pre == 0xDD) 
		{
            r  = gRegIX;
            rp = gRegPairsIX;
            rp2 = gRegPairs2IX;
        }
        else if (pre == 0xFD) 
		{
            r  = gRegIY;
            rp = gRegPairsIY;
            rp2 = gRegPairs2IY;
        }
    }
    
    /* parse the opcode */
    uint8_t x = (op >> 6) & 3;	// top 2 bits: 6&7
    uint8_t y = (op >> 3) & 7;	// middle bits: 3,4 & 5
    uint8_t z = op & 7;			// lower 3 bits: 0,1 & 2
    uint8_t p = y >> 1;
    uint8_t q = y & 1;
    if (x == 1) 
	{
        /* 8-bit load block */
        if (y == 6) 
		{
            if (z == 6) 
			{
                /* special case LD (HL),(HL) */
            }
            else 
			{
                /* LD (HL),r; LD (IX+d),r; LD (IY+d),r */
                if (pre && ((z == 4) || (z == 5))) 
				{
                    /* special case LD (IX+d),L/H (don't use IXL/IXH) */
					inst.RegFlags = gReg[z] | r[6];
                }
                else 
				{
					inst.RegFlags = r[z] | r[6];
                }
            }
        }
        else if (z == 6) 
		{
            /* LD r,(HL); LD r,(IX+d); LD r,(IY+d) */
            if (pre && ((y == 4) || (y == 5))) 
			{
                /* special case LD H/L,(IX+d) (don't use IXL/IXH) */
				inst.RegFlags = gReg[y] | r[6];
            }
            else 
			{
				sprintf_s(tempStr, "Load %s from memory location (HL)", GetRegName(r[y]).c_str()/*, NumStr(CPUIF->ReadByte(pc))*/);
				inst.RegFlags = r[y] | r[6];
            }
		}
        else 
		{
            /* regular LD r,s */
			inst.RegFlags = gReg[y] | gReg[z];
        }
    }
    else if (x == 2) 
	{
		/* ADD A,s, ADC A,s, SUB s, SBC A,s, AND s, XOR s, OR s, CP s */
		/* Where s is (HL)/(IX+d)/(IX+d) or r */
		switch (y)
		{
			case 0: // ADD A, r 
				sprintf_s(tempStr, "Add %s to A. Result stored in A.", GetRegName(r[z]).c_str());
				break;
			case 1: // ADC A, s 
				sprintf_s(tempStr, "Add with Carry. %s and Carry flag added to A. Result stored in A.", GetRegName(r[z]).c_str());
				inst.RegFlags |= Z80Reg::F;
				break;
			case 2: // SUB s
				sprintf_s(tempStr, "Subtract %s from A. Result stored in A.", GetRegName(r[z]).c_str());
				break;
			case 3: // SBC A, s
				sprintf_s(tempStr, "Subtract with Carry. %s and C flag are subtracted from A. Result stored in A.", GetRegName(r[z]).c_str());
				inst.RegFlags |= Z80Reg::F;
				break;
			case 4: // AND s
				sprintf_s(tempStr, "Logical AND A with %s. Result stored in A.", GetRegName(r[z]).c_str());
				break;
			case 5: // XOR s
				sprintf_s(tempStr, "Exclusive OR A with %s. Result stored in A.", GetRegName(r[z]).c_str());
				break;
			case 6: // OR s
				sprintf_s(tempStr, "Logical OR A with %s. Result stored in A.", GetRegName(r[z]).c_str());
				break;
			case 7: // CP s
				std::string regName = GetRegName(r[z]);
				sprintf_s(tempStr, "Compare A with %s. Subtract %s from A but discard result.\n\n"
					"Unsigned\nA == %s: Z flag is set.\nA != %s: Z flag is reset.\n"
					"A < %s:  C flag is set.\nA >= %s: C flag is reset.\n\n"
					"Signed\nA == %s: Z flag is set.\nA != %s: Z flag is reset.\n"
					"A < %s:  S and P/V are different.\nA >= %s: S and P/V are the same.\n", 
					regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str(), regName.c_str());
				inst.RegFlags |= Z80Reg::F;
				break;
		}
		inst.RegFlags |= r[z] | Z80Reg::A;
    }
    else if (x == 0) 
	{
        switch (z) 
		{
            case 0:
                switch (y) 
				{
                    case 0: /* NOP */
						break;
                    case 1: /* EX AF,AF'*/
						inst.RegFlags = Z80Reg::AF; // todo inst.RegFlags = AF'?
						break;
                    case 2: /* DJNZ*/
						inst.RegFlags = Z80Reg::B;
						break;
                    case 3: /* JR e*/
						break;
                    default: /* JR C/NC/Z/NZ, e*/
						inst.RegFlags = Z80Reg::F;
						break;
                }
                break;
            case 1:
                if (q == 0) /* LD dd,nn*/
				{
					sprintf_s(tempStr, "Load %s with %s", GetRegName(rp[p]).c_str(), NumStr(CPUIF->ReadWord(pc)));
					inst.RegFlags = rp[p];
				}
                else /* ADD HL, ss. ADD IX, pp. ADD IY, rr. */
					inst.RegFlags = rp[2] | rp[p];
				break;
            case 2: 
                {
                    switch (y) 
					{
                        case 0: /* LD (BC),A */
							inst.RegFlags = Z80Reg::A | Z80Reg::BC_Indirect;
							break;
                        case 1: /* LD A,(BC) */
							inst.RegFlags = Z80Reg::A | Z80Reg::BC_Indirect;
							break;
                        case 2: /* LD (DE),A*/
							inst.RegFlags = Z80Reg::A | Z80Reg::DE_Indirect;
							break;
                        case 3: /* LD A,(DE)*/
							inst.RegFlags = Z80Reg::A | Z80Reg::DE_Indirect;
							break;
                        case 4: /* LD (nn), HL*/
						{
							const uint16_t nn = CPUIF->ReadWord(pc);
							sprintf_s(tempStr, "Load the memory locations %s and %s from HL", NumStr(nn), NumStr(uint16_t(nn+1)));
							inst.RegFlags = rp[p];
							break;
						}
                        case 5: /* LD HL,(nn)*/
						{
							const uint16_t nn = CPUIF->ReadWord(pc);
							sprintf_s(tempStr, "Load HL from memory locations %s and %s", NumStr(nn), NumStr(uint16_t(nn+1)));
							inst.RegFlags = rp[p];
							break;
						}
                        case 6: /* LD (nn), A*/
							inst.RegFlags = Z80Reg::A;
							break;
                        case 7: /* LD A, (n)*/
							inst.RegFlags = Z80Reg::A;
							break;
                    }
                }
                break;
            case 3: /* INC ss (register pair)*/
				inst.RegFlags = rp[p];
				break;
            case 4: /* INC (HL)/(IX+d)/(IY+d) or r */
			{
				z80_t* pCPU = (z80_t *)CPUIF->GetCPUEmulator();
				if (y == 6) /* (HL)/(IX+d)/(IY+d) */
				{
					if (pre)
					{
						const uint16_t addr = (pre == 0xdd ? z80_ix(pCPU) : z80_iy(pCPU)) + CPUIF->ReadByte(pc);
						sprintf_s(tempStr, "Increment indexed memory location %s", NumStr(addr));
					}
					else
					{
						sprintf_s(tempStr, "Increment byte at memory location %s", GetRegName(r[y]).c_str());
					}
				}
				else
				{
					sprintf_s(tempStr, "Increment %s", GetRegName(r[y]).c_str());
				}
				inst.RegFlags = r[y];
				break;
			}
            case 5: /* DEC (HL)/(IX+d)/(IY+d) or r */
			{
				z80_t* pCPU = (z80_t *)CPUIF->GetCPUEmulator();
				if (y == 6) /* (HL)/(IX+d)/(IY+d) */
				{
					if (pre)
					{
						const int8_t byteVal = CPUIF->ReadByte(pc);
						const uint16_t addr = (pre == 0xdd ? z80_ix(pCPU) : z80_iy(pCPU)) + byteVal;
						sprintf_s(tempStr, "Decrement byte at indexed memory location %s", NumStr(addr));
					}
					else
					{
						sprintf_s(tempStr, "Decrement byte at memory location %s", GetRegName(r[y]).c_str());
					}
				}
				else
				{
					sprintf_s(tempStr, "Decrement %s", GetRegName(r[y]).c_str());
				}
				inst.RegFlags = r[y];
				break;
			}
            case 6: /* LD s, n. Where s is (HL)/(IX+d)/(IY+d) or r*/
				if (y == 6)
					sprintf_s(tempStr, "Load %s into the memory location %s", NumStr(CPUIF->ReadByte(pc)), GetRegName(r[y]).c_str());
				else
					sprintf_s(tempStr, "Load register %s with %s", GetRegName(r[y]).c_str(), NumStr(CPUIF->ReadByte(pc)));

				inst.RegFlags = r[y];
				break;
            case 7: 
				if (y < 6) /* RLCA, RRCA, RLA, RRA, DAA, CPL, */
					inst.RegFlags = Z80Reg::A;
				else /* SCF, CCF*/
					inst.RegFlags = Z80Reg::F;
				break;
        }
    }
    else 
	{
        switch (z) 
		{
            case 0: /* RET NZ/Z/NC/C/PO/PE/P/M*/
				inst.RegFlags = Z80Reg::F;
				break;
            case 1:
                if (q == 0) /* POP qq (register pair)*/
				{
					sprintf_s(tempStr, "Pop %s from stack", GetRegName(rp2[p]).c_str());
					inst.RegFlags = rp2[p];
				}
                else 
				{
                    switch (p) 
					{
                        case 0: /* RET*/
							break;
                        case 1: /* EXX */
							break;
                        case 2: /* JP (HL)/(IX)/(IY)*/
							inst.RegFlags = rp[2];
							break;
                        case 3: /* LD SP HL/IX/IY*/
							inst.RegFlags = rp[p];
							break;
                    }
                }
                break;
            case 2: /* JP cc, nn. Where cc is NZ/Z/NC/C/PO/PE/P/M*/
				inst.RegFlags = Z80Reg::F;
				break;
            case 3:
                switch (y) 
				{
                    case 0: /* JP nn*/
						break;
                    case 2: /* OUT (n), A*/
						inst.RegFlags = Z80Reg::A;
						break;
                    case 3: /* IN A, (n)*/
						inst.RegFlags = Z80Reg::A;
						break;
                    case 4: /* EX (SP), HL/IX/IY*/
						inst.RegFlags = rp[2] | Z80Reg::SP_Indirect;
						break;
                    case 5: /* EX DE,HL*/
						inst.RegFlags = Z80Reg::DE | Z80Reg::HL;
						break;
                    case 6: /* DI*/
						break;
                    case 7: /* EI*/
						break;
                    case 1: /* CB prefix */
                        if (pre) 
							d = CPUIF->ReadByte(pc++);
						op = CPUIF->ReadByte(pc++);

						//const char* oct = "01234567";

                        x = (op >> 6) & 3;
                        y = (op >> 3) & 7;
                        z = op & 7;
                        if (x == 0) 
						{
							/* rot and shift instructions */
							/* RLC, RRC, RL, RR, SLA, SRA, SLL, SRL */
							switch (y)
							{
								case 0: // RLC 
									if (z==6) // (HL)
										sprintf_s(tempStr, "Rotate (HL) left 1 bit. Bit 7 is copied to C flag and to bit 0.");
									break;
								case 1: // RRC
									if (z==6) // (HL)
										sprintf_s(tempStr, "Rotate (HL) right 1 bit. Bit 0 is copied to C flag and to bit 7.");
									break;
								case 2: // RL 
									break;
								case 3: // RR
									break;
								case 4: // SLA
									break;
								case 5: // SRA
									break;
								case 6: // SLL
									break;
								case 7: // SRL
									break;
							}
                            
							inst.RegFlags = r[z];
                        }
                        else if (x == 1)
						{
							/*if (pre) {
                                _CHR(','); _Md(d);
                            }
                            if (!pre || (z != 6)) {
                                _CHR(','); _STR(r[z]);
                            }*/

							// BIT
							// todo: finish this
							sprintf_s(tempStr, "Test bit %d of %s", y, GetRegName(r[z]).c_str());

						}
                        else if (x == 2)
						{
							// RES
						}
						else if (x == 3)
						{
							// SET
						}
						inst.RegFlags = r[z];
                        break;
                }
                break;
            case 4: /* CALL cc, nn. Where cc is NZ/Z/NC/C/PO/PE/P/M*/
				inst.RegFlags = Z80Reg::F;
				break;
            case 5: 
                if (q == 0) 
				{
                    /* PUSH qq (register pair)*/
					sprintf_s(tempStr, "Push %s onto stack", GetRegName(rp2[p]).c_str());
					inst.RegFlags = rp2[p];
                }
                else 
				{
                    switch (p) 
					{
                        case 0: /* CALL nn*/
							break;
                        case 1: /* DBL PREFIX*/
							break;
                        case 3: /* DBL PREFIX*/
							break;
                        case 2: /* ED prefix */
							op = CPUIF->ReadByte(pc++);
                            x = (op >> 6) & 3;
                            y = (op >> 3) & 7;
                            z = op & 7;
                            p = y >> 1;
                            q = y & 1;
                            if ((x == 0) || (x == 3)) 
							{
                                /* NOP (ED)*/
                            }
                            else if (x == 2) 
							{
                                if ((y >= 4) && (z <= 3)) 
								{
                                    /* block instructions */
									/* LDI,	 CPI,  INI,  OUTI */
									/* LDD,  CPD,  IND,  OUTD */
									/* LDIR, CPIR, INIR, OTIR */
									/* LDDR, CPDR, INDR, OTDR */
									 
									if (z == 0) /* First column. LDI, LDD, LDIR, LDDR*/
									{
										switch (y-4)
										{
											case 0:
												sprintf_s(tempStr, "LDI");
												break;
											case 1:
												sprintf_s(tempStr, "LDD");
												break;
											case 2:
												sprintf_s(tempStr, "LDIR");
												break;
											case 3:
												sprintf_s(tempStr, "LDDR");
												break;
										}
										inst.RegFlags = Z80Reg::BC | Z80Reg::HL | Z80Reg::DE |Z80Reg::DE_Indirect | Z80Reg::HL_Indirect;
									}
									else if (z == 1) /* Second column. CPI, CPDR, CPD, CPIR*/
									{
										inst.RegFlags = Z80Reg::A | Z80Reg::BC | Z80Reg::HL | Z80Reg::HL_Indirect;
									}
									else
									{
										/* Last 2 columns. OUTI, INI, OUTD, IND, INIR, OTIR, INDR, OTDR*/
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
										if(y!=6)
											inst.RegFlags = r[y] | Z80Reg::C; 
										else
											inst.RegFlags = Z80Reg::C;
										break;
                                    case 1: /* OUT (C), r*/
										if(y!=6)
											inst.RegFlags = r[y] | Z80Reg::C;
										else
											inst.RegFlags = Z80Reg::C;
										break;
                                    case 2: /* SBC HL, ss. ADC HL, ss. Where ss is BC/DE/HL/SP*/
										if (q == 0)
											sprintf_s(tempStr, "Subtract with Carry. %s and C flag are subtracted from HL. Result stored in HL.", GetRegName(rp[p]).c_str());
										else
											sprintf_s(tempStr, "Add with Carry. %s and C flag are added to HL. Result stored in HL.", GetRegName(rp[p]).c_str());
										inst.RegFlags = Z80Reg::HL | rp[p];
										break;
                                    case 3: /* LD (nn), dd. LD dd, (nn). */
									{
										const uint16_t nn = CPUIF->ReadWord(pc);
										if (q==0)
											sprintf_s(tempStr, "Load memory locations (%s) and (%s) from %s", NumStr(nn), NumStr(uint16_t(nn+1)), GetRegName(rp[p]).c_str());
										else
											sprintf_s(tempStr, "Load %s from memory locations (%s) and (%s)", GetRegName(rp[p]).c_str(), NumStr(nn), NumStr(uint16_t(nn+1)));
										inst.RegFlags =  rp[p];
										break;
									}
                                    case 4: /* NEG */
										inst.RegFlags =  Z80Reg::A;
										break;
                                    case 5: /* RETI/RETN*/
										break;
                                    case 6: /* IM 0/1/2*/
										break;
                                    case 7: /* LD I,A, LD R,A, LD A,I, LD A,R, RRD, RLD, NOP (ED), NOP (ED)*/
										if (y == 0 || y == 2)
											inst.RegFlags =  Z80Reg::A | Z80Reg::I;
										else
										if (y == 1 || y == 3)
											inst.RegFlags =  Z80Reg::A | Z80Reg::R;
										else
											inst.RegFlags = Z80Reg::A | Z80Reg::HL_Indirect;
										break;
                                }
                            }
                            break;
                    }
                }
                break;
            case 6: /* ADD A,n, ADC A,n, SUB n, SBC A,n, AND n, XOR n, OR n, CP n*/
				inst.RegFlags =  Z80Reg::A;
				break;
            case 7: /* RST */ 
				// todo
				break;
        }
    }
	inst.Description = tempStr;
}

// TODO: Eventually fill this up
// start with tests & instructions not obvious

InstructionInfoMap g_InstructionInfo =
{
	{0x07, {"RLCA: A is rotated left 1 bit. The sign bit (bit 7) is copied to the Carry flag and also to bit 0."}},	//RLCA
	{0x0F, {"RRCA: A is rotated right 1 bit. Bit 0 is copied to the Carry flag and also to bit 7"}},	//RRCA
	{0x10, {"DJNZ: Decrement B & Jump relative if it isn't 0"}},	//DJNZ
	{0x2F, {"CPL: Complement(inverted) bits of A"} },	//CPL
	{0x76, {"HALT: Suspend CPU until interrupt or reset occurs."} },	//HALT
	{0x96, {"Subtract (HL) from A"}},	// SUB (HL)
	{0xD9, {"EXX: Exchange BC, DE & HL with shadow registers" }},	// EXX
};

// extended instructions
InstructionInfoMap g_InstructionInfo_ED =
{
	{0xB0, {"LDIR: Transfer BC bytes from address pointed to by HL to address pointed to by DE.\nHL & DE get incremented, BC gets decremented."}},//LDIR
	{0xB8, {"LDDR: Transfer BC bytes from address pointed to by HL to address pointed to by DE.\nHL, DE & BC get decremented."}},//LDDR
	{0x67, {"Rotate (HL) right 4 bits through lower nybble of A"}},	// rrd
	{0x6F, {"Rotate (HL) left 4 bits through lower nybble of A"}}, // rld
};

// bit instructions
InstructionInfoMap g_InstructionInfo_CB =
{
	{0x07,{"Rotate A Left with Carry. Bit 7 goes to Carry & bit 0."}},	//RLC A
};

// IX/IY instructions
InstructionInfoMap g_InstructionInfo_Index =
{
	{0x09, {"Add BC to %s"}},	// Add IX/IY,BC
};

// IX/IY bit instructions
InstructionInfoMap g_InstructionInfo_IndexBit =
{
	{0x00, {"Rotate (%s + *) left with carry B bits"}},	// rlc (ix+*),b
};

// Get a description for the function from the tables
const FInstructionInfo* GetInstructionInfo(FCodeAnalysisState& state, const FCodeInfo* pCodeInfo)
{
	uint8_t instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address);
	InstructionInfoMap::const_iterator it = g_InstructionInfo.end();
	//const char* pRegName = nullptr;
	//uint32_t regs = 0;

	switch (instrByte)
	{
	case 0xED:
		instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 1);
		it = g_InstructionInfo_ED.find(instrByte);
		if (it == g_InstructionInfo_ED.end())
			return nullptr;

		//regs = it->second.Registers;
		break;

	case 0xCB:
		instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 1);
		it = g_InstructionInfo_CB.find(instrByte);
		if (it == g_InstructionInfo_CB.end())
			return nullptr;
		//regs = it->second.Registers;
		break;

	case 0xDD:
	case 0xFD:
		//pRegName = instrByte == 0xDD ? "IX" : "IY";
		//regs = instrByte == 0xDD ? Z80Reg::IX : Z80Reg::IY;
		instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 1);
		if (instrByte == 0xCB)	// bit instructions
		{
			instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 2);
			it = g_InstructionInfo_IndexBit.find(instrByte);
			if (it == g_InstructionInfo_IndexBit.end())
				return nullptr;
			//regs |= it->second.Registers;
		}
		else
		{
			it = g_InstructionInfo_Index.find(instrByte);
			if (it == g_InstructionInfo_Index.end())
				return nullptr;
			//regs |= it->second.Registers;
		}
		break;

	default:
		it = g_InstructionInfo.find(instrByte);
		if (it == g_InstructionInfo.end())
			return nullptr;
		//regs = it->second.Registers;
		break;
	}

	return &it->second;
}

void ShowCodeToolTipZ80(FCodeAnalysisState& state, const FCodeInfo* pCodeInfo)
{
	std::string desc;

	// Get flags for register usage and try to auto generate a description for the instruction.
	FAutoGenInstructionInfo genInstrInfo;
	GetFlagsAndGenerateDescriptionFromOpcode(pCodeInfo->Address, state.CPUInterface, genInstrInfo);

	if (genInstrInfo.Description.empty()) // fall back to LUT if there's no desc generated
	{
		const FInstructionInfo* pInstructionInfo = GetInstructionInfo(state, pCodeInfo);

		if (pInstructionInfo != nullptr)
			desc = std::string(pInstructionInfo->Description);
	}
	else
	{
		desc = genInstrInfo.Description;
	}
	
	std::string regStr = GenerateRegisterValueString(genInstrInfo.RegFlags, state.CPUInterface);

	if (!genInstrInfo.RegFlags && desc.empty())
		return;

	ImGui::BeginTooltip();
	if (!desc.empty())
		ImGui::Text(desc.c_str());	// Instruction description
	ImGui::Separator();
	ImGui::Text(regStr.c_str());
	ImGui::EndTooltip();
}
