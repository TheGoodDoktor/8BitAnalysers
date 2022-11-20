#include "CodeToolTipsZ80.h"
#include "../CodeToolTips.h"
#include "../CodeAnalyser.h"

#include <Util/Misc.h>
#include <chips/z80.h>

#include "imgui.h"

#include <map>

// register bits
namespace Z80Reg
{
	const uint32_t	A = 0x00000001;
	const uint32_t	B = 0x00000002;
	const uint32_t	C = 0x00000004;
	const uint32_t	D = 0x00000008;
	const uint32_t	E = 0x00000010;
	const uint32_t	H = 0x00000020;
	const uint32_t	L = 0x00000040;
	const uint32_t	SP = 0x00000080;
	const uint32_t	BC = 0x00000100;
	const uint32_t	DE = 0x00000200;
	const uint32_t	HL = 0x00000400;
	const uint32_t	IX = 0x00000800;
	const uint32_t	IY = 0x00001000;
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

	if (Regs & Z80Reg::SP)
	{
		sprintf_s(tempStr, "SP = %s ", NumStr(z80_sp(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::BC)
	{
		sprintf_s(tempStr, "BC = %s ", NumStr(z80_bc(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::DE)
	{
		sprintf_s(tempStr, "DE = %s ", NumStr(z80_de(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::HL)
	{
		sprintf_s(tempStr, "HL = %s ", NumStr(z80_hl(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IX)
	{
		sprintf_s(tempStr, "IX = %s ", NumStr(z80_ix(pCPU)));
		outString += std::string(tempStr);
	}

	if (Regs & Z80Reg::IY)
	{
		sprintf_s(tempStr, "IY = %s ", NumStr(z80_iy(pCPU)));
		outString += std::string(tempStr);
	}

	return outString;
}

// TODO: Eventually fill this up
// start with tests & instructions not obvious

InstructionInfoMap g_InstructionInfo =
{
	{0x10, {"DJNZ: Decrement B & Jump relative if it isn't 0", Z80Reg::B}},	//DJNZ
	{0x2F, {"CPL: Complement(inverted) bits of A" , Z80Reg::A} },	//CPL
	{0x96, {"Subtract (HL) from A", Z80Reg::HL | Z80Reg::A}}	// SUB (HL)
};

// extended instructions
InstructionInfoMap g_InstructionInfo_ED =
{
	{0xB0, {"LDIR: Transfer BC bytes from address pointed to by HL to address pointed to by DE.\nHL & DE get incremented, BC gets decremented.",Z80Reg::BC | Z80Reg::HL | Z80Reg::DE}},//LDIR
	{0xB8, {"LDDR: Transfer BC bytes from address pointed to by HL to address pointed to by DE.\nHL, DE & BC get decremented.",Z80Reg::BC | Z80Reg::HL | Z80Reg::DE}},//LDDR
	{0x67, {"Rotate (HL) right 4 bits through lower nybble of A",Z80Reg::A}},	// rrd
	{0x6F, {"Rotate (HL) left 4 bits through lower nybble of A",Z80Reg::A}}, // rld
};

// bit instructions
InstructionInfoMap g_InstructionInfo_CB =
{
	{0x07,{"Rotate A Left with Carry. Bit 7 goes to Carry & bit 0.", Z80Reg::A}},	//RLC A
};

// IX/IY instructions
InstructionInfoMap g_InstructionInfo_Index =
{
	{0x09, {"Add BC to %s",Z80Reg::BC}},	// Add IX/IY,BC
};

// IX/IY bit instructions
InstructionInfoMap g_InstructionInfo_IndexBit =
{
	{0x00, {"Rotate (%s + *) left with carry B bits",Z80Reg::B}},	// rlc (ix+*),b
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

// Sam : you can write this one, return an empty string if you can't generate the string
std::string GenerateInstructionDescription(FCodeAnalysisState& state, const FCodeInfo* pCodeInfo)
{
	std::string desc;

	return desc;
}

void ShowCodeToolTipZ80(FCodeAnalysisState& state, const FCodeInfo* pCodeInfo)
{
	std::string desc = GenerateInstructionDescription(state, pCodeInfo);
	if (desc.empty())	// fall back to LUT is there's no desc generated
	{
		const FInstructionInfo* pInstructionInfo = GetInstructionInfo(state, pCodeInfo);

		if (pInstructionInfo != nullptr)
			desc = std::string(pInstructionInfo->Description);
	}

	//std::string regStr = GenerateRegisterValueString(regs, state.CPUInterface);

	ImGui::BeginTooltip();
	ImGui::Text(desc.c_str());	// Instruction description
	//ImGui::Text(regStr.c_str(), pRegName);	// TODO: Sam put your reg stuff here..
	ImGui::EndTooltip();
}