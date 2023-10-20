#include "CodeToolTips6502.h"
#include "../../CodeAnalyser.h"
#include "../CodeToolTips.h"

#include <imgui.h>

struct FInstrInfo
{
	std::string		Desc;
};

/*
		PHP	PLP	PHA	PLA	DEY	TAY	INY	INX
		08	28	48	68	88	A8	C8	E8
		CLC	SEC	CLI	SEI	TYA	CLV	CLD	SED
		18	38	58	78	98	B8	D8	F8
		TXA	TXS	TAX	TSX	DEX	NOP
		8A	9A	AA	BA	CA	EA
*/

std::unordered_map<uint8_t, FInstrInfo> g_SingleByteInstructions = 
{
	{0x00, {"Trigger software interrupt"}},					// BRK
	{0x08, {"Push processor status register"}},		// PHP
	{0x28, {"Pull processor status register"}},		// PLP
	{0x48, {"Push A onto stack"}},					// PHA
	{0x68, {"Pull A from stack"}},					// PLA
	{0x88, {"Decrement Y"}},						// DEY
	{0xA8, {"Transfer A to Y"}},					// TAY
	{0xC8, {"Increment Y"}},						// INY
	{0xE8, {"Increment X"}},						// INX
	{0x18, {"Clear carry flag"}},					// CLC
	{0x38, {"Set carry flag"}},						// SEC
	{0x58, {"Clear interrupt flag (disable interrupts)"}},				// CLI
	{0x78, {"Set interrupt flag (enable interrupts)"}},					// SEI
	{0x98, {"Transfer Y to A"}},					// TYA
	{0xB8, {"Clear overflow flag"}},				// CLV
	{0xD8, {"Clear decimal flag (disable BCD)"}},	// CLD
	{0xF8, {"Set decimal flag (enable BCD)"}},		// SED
	{0x8A, {"Transfer X to A"}},					// TXA
	{0x9A, {"Transfer X to SP"}},					// TXS
	{0xAA, {"Transfer A to X"}},					// TAX
	{0xBA, {"Transfer SP to X"}},					// TSX
	{0xCA, {"Decrement X"}},						// DEX
	{0xEA, {"No operation"}},						// NOP
	{0x40, {"Return from Interrupt"}},				// RTI
	{0x60, {"Return from Subroutine"}},				// RTS
};

bool OutputTooltipSingleByteInstruction(const uint8_t opcode)
{
	const auto& sbiDescIt = g_SingleByteInstructions.find(opcode);
	if(sbiDescIt == g_SingleByteInstructions.end())
		return false;
	const FInstrInfo& desc = sbiDescIt->second;

	ImGui::Text("%s",desc.Desc.c_str());
	return true;
}


bool OutputTooltipBranchInstruction(FCodeAnalysisState& state, uint16_t addr, const uint8_t opcode)
{
	static const char* flagNames[] = { "negative","overflow","carry","zero" };

	if((opcode & 0b11111) != 0b10000)
		return false;

	ImGui::Text("Branch on %s flag %s", flagNames[opcode >> 6], opcode & 0b00100000 ? "set" : "clear");
	return true;
}

void OutputTooltipGroupOne(FCodeAnalysisState& state, uint16_t addr, const uint8_t opcode)
{
	const uint8_t addressMode = (opcode >> 2) & 7;
	const uint8_t instruction = (opcode >> 5) & 7;

	switch (instruction)
	{
	case 0b000:	//	ORA
		ImGui::Text("OR A");
		break;
	case 0b001:	//	AND
		ImGui::Text("AND A");
		break;
	case 0b010:	//	EOR
		ImGui::Text("EOR A");
		break;
	case 0b011:	//	ADC
		ImGui::Text("Add A with carry");
		break;
	case 0b100:	//	STA
		ImGui::Text("Store A");
		break;
	case 0b101:	//	LDA
		ImGui::Text("Load A");
		break;
	case 0b110:	//	CMP
		ImGui::Text("Compare A");
		break;
	case 0b111:	//	SBC
		ImGui::Text("Subtract A with carry");
		break;
	}

	switch (addressMode)
	{
		case 0b000: // (zero page, X)
			break;
		case 0b001: // zero page
		{
			const uint8_t zeroPage = state.ReadByte(addr + 1);

			ImGui::Text("with zero page entry %s",NumStr(zeroPage));
		}
		break;
		case 0b010: // #immediate
		{
			const uint8_t intermediate = state.ReadByte(addr + 1);

			ImGui::Text("with %s", NumStr(intermediate));
		}
		break;
		case 0b011: // absolute
		{

		}
		break;

		case 0b100: // (zero page), Y
			break;
		case 0b101: // zero page, X
			break;
		case 0b110: // absolute, Y
			break;
		case 0b111: // absolute, X
			break;
	}
}

void OutputTooltipGroupTwo(FCodeAnalysisState& state, uint16_t addr, const uint8_t opcode)
{
	const uint8_t addressMode = (opcode >> 2) & 7;
	const uint8_t instruction = (opcode >> 5) & 7;

	char addressModeDesc[64];

	switch (addressMode)
	{
	case 0b000://	#immediate
		snprintf(addressModeDesc, sizeof(addressModeDesc), "%s", NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b001://	zero page
		snprintf(addressModeDesc, sizeof(addressModeDesc), "zero page %s",NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b010://	accumulator
		snprintf(addressModeDesc,sizeof(addressModeDesc), "A");
		break;
	case 0b011://	absolute
		snprintf(addressModeDesc, sizeof(addressModeDesc), "%s", NumStr(state.ReadWord(addr + 1)));
		break;
	case 0b101://	zero page, X
		snprintf(addressModeDesc, sizeof(addressModeDesc), "zero page %s, x", NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b111://	absolute, X
		snprintf(addressModeDesc, sizeof(addressModeDesc), "%s, x", NumStr(state.ReadWord(addr + 1)));
		break;
	}

	switch (instruction)
	{
		case 0b000://	ASL
			ImGui::Text("Arithmetic shift left %s", addressModeDesc);
			break;
		case 0b001://	ROL
			ImGui::Text("Bit rotate left %s", addressModeDesc);
			break;
		case 0b010://	LSR
			ImGui::Text("Logical shift right %s", addressModeDesc);
			break;
		case 0b011://	ROR
			ImGui::Text("Bit rotate right %s", addressModeDesc);
			break;
		case 0b100://	STX
			ImGui::Text("Store X to %s", addressModeDesc);
			break;
		case 0b101://	LDX
			ImGui::Text("Load X with %s", addressModeDesc);
			break;
		case 0b110://	DEC
			ImGui::Text("Decrement %s", addressModeDesc);
			break;
		case 0b111://	INC
			ImGui::Text("Increment %s", addressModeDesc);
			break;
	}

	
}

void OutputTooltipGroupThree(FCodeAnalysisState& state, uint16_t addr, const uint8_t opcode)
{
	const uint8_t addressMode = (opcode >> 2) & 7;
	const uint8_t instruction = (opcode >> 5) & 7;

	switch (instruction)
	{
		case 0b001://	BIT
			ImGui::Text("Test bit");
			break;
		case 0b010://	JMP
			ImGui::Text("Jump");
			break;
		case 0b011://	JMP(abs)
			ImGui::Text("Jump");
			break;
		case 0b100://	STY
			ImGui::Text("Store Y");
			break;
		case 0b101://	LDY
			ImGui::Text("Load Y");
			break;
		case 0b110://	CPY
			ImGui::Text("Compare Y");
			break;
		case 0b111://	CPX
			ImGui::Text("Compare X");

			break;
	}
		
	//The addressing modes are the same as the 10 case, except that accumulator mode is missing.

	switch (addressMode)
	{
		case 0b000://	#immediate
			break;
		case 0b001://	zero page
			break;
		case 0b011://	absolute
			break;
		case 0b101://	zero page, X
			break;
		case 0b111://	absolute, X
			break;
	}
}

void OutputInstructionTooltip(FCodeAnalysisState& state, uint16_t addr)
{
	const uint8_t opcode = state.ReadByte(addr);
	const uint8_t group = opcode & 3;
	
	if(OutputTooltipSingleByteInstruction(opcode))
		return;

	if(OutputTooltipBranchInstruction(state,addr,opcode))
		return;

	switch (opcode)
	{
	// handle specifics first
	case 0x20:	// JSR
		ImGui::Text("Jump to subroutine at %s",NumStr(state.ReadWord(addr+1)));
		break;
	
	default:
		if (group == 1)
			OutputTooltipGroupOne(state, addr, opcode);
		else if (group == 2)
			OutputTooltipGroupTwo(state, addr, opcode);
		else if (group == 0)
			OutputTooltipGroupThree(state, addr, opcode);
		else
			ImGui::Text("Not Implemented");
	}

}

void ShowCodeToolTip6502(FCodeAnalysisState& state, uint16_t addr)
{
	ImGui::BeginTooltip();
	OutputInstructionTooltip(state,addr);
	ImGui::EndTooltip();
}