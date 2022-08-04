#include "CodeToolTips.h"
#include "imgui.h"
#include "Speccy/Speccy.h"
#include <map>
#include "CodeAnalyser.h"

// TODO: Eventually fill this up
// start with tests & instructions not obvious

typedef std::map<uint8_t, const char *> InstructionInfoMap;

InstructionInfoMap g_InstructionInfo =
{
	{0x10, "DJNZ: Decrement B & Jump relative if it isn't 0"},	//DJNZ
	{0x2F, "CPL: Complement(inverted) bits of A"},	//CPL
	{0x96, "Subtract (HL) from A"},	// SUB (HL)
};

// extended instructions
InstructionInfoMap g_InstructionInfo_ED =
{
	{0xB0,"LDIR: Transfer BC bytes from address pointed to by HL to address pointed to by DE.\nHL & DE get incremented, BC gets decremented."},//LDIR
	{0xB8,"LDDR: Transfer BC bytes from address pointed to by HL to address pointed to by DE.\nHL, DE & BC get decremented."},//LDIR
	{0x67, "Rotate (HL) right 4 bits through lower nybble of A"},	// rrd
	{0x6F, "Rotate (HL) left 4 bits through lower nybble of A"}, // rld
};

// bit instructions
InstructionInfoMap g_InstructionInfo_CB =
{
	{0x07,"Rotate A Left with Carry. Bit 7 goes to Carry & bit 0."},	//RLC A
};

// IX/IY instructions
InstructionInfoMap g_InstructionInfo_Index =
{
	{0x09, "Add BC to %s"},	// Add IX/IY,BC
};

// IX/IY bit instructions
InstructionInfoMap g_InstructionInfo_IndexBit =
{
	{0x00, "Rotate (%s + *) left with carry B bits"},	// rlc (ix+*),b
};

void ShowCodeToolTipZ80(FCodeAnalysisState &state, const FCodeInfo *pCodeInfo)
{
	uint8_t instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address);
	InstructionInfoMap::const_iterator it = g_InstructionInfo.end();
	const char *pRegName = nullptr;

	switch (instrByte)
	{
	case 0xED:
		instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 1);
		it = g_InstructionInfo_ED.find(instrByte);
		if (it == g_InstructionInfo_ED.end())
			return;
		break;

	case 0xCB:
		instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 1);
		it = g_InstructionInfo_CB.find(instrByte);
		if (it == g_InstructionInfo_CB.end())
			return;
		break;

	case 0xDD:
	case 0xFD:
		pRegName = instrByte == 0xDD ? "IX" : "IY";
		instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 1);
		if (instrByte == 0xCB)	// bit instructions
		{
			instrByte = state.CPUInterface->ReadByte(pCodeInfo->Address + 2);
			it = g_InstructionInfo_IndexBit.find(instrByte);
			if (it == g_InstructionInfo_IndexBit.end())
				return;
		}
		else
		{
			it = g_InstructionInfo_Index.find(instrByte);
			if (it == g_InstructionInfo_Index.end())
				return;
		}
		break;

	default:
		it = g_InstructionInfo.find(instrByte);
		if (it == g_InstructionInfo.end())
			return;
		break;
	}

	ImGui::BeginTooltip();
	ImGui::Text(it->second, pRegName);
	ImGui::EndTooltip();
}

void ShowCodeToolTip(FCodeAnalysisState& state, const FCodeInfo* pCodeInfo)
{
	if (state.CPUInterface->CPUType == ECPUType::Z80)
		ShowCodeToolTipZ80(state, pCodeInfo);
}