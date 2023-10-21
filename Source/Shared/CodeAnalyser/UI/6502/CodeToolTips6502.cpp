#include "CodeToolTips6502.h"
#include "../../CodeAnalyser.h"
#include "../CodeToolTips.h"

#include <imgui.h>

// Descriptions got from
// https://www.masswerk.at/6502/6502_instruction_set.html

struct FInstrInfo
{
	std::string		Desc;
};

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

enum class EAddressMode
{
	Accumulator,
	Immediate,
	Absolute,
	AbsoluteIndexed,
	Indirect,
	ZeroPage,
	ZeroPageIndexed,			// zp,X
	ZeroPageIndexedIndirect,	// (zp,X)
	ZeroPageIndirectIndexed,	// (zp),X
};

std::string GenerateAddressModeDescription(FCodeAnalysisState& state, uint16_t addr, EAddressMode addressMode)
{
	std::string desc;

	return desc;
}

void OutputTooltipGroupOne(FCodeAnalysisState& state, uint16_t addr, const uint8_t opcode)
{
	const uint8_t addressMode = (opcode >> 2) & 7;
	const uint8_t instruction = (opcode >> 5) & 7;

	char addressModeDesc[64];

	switch (addressMode)
	{
		case 0b000: // (zero page, X)
			snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of (zero page %s + X)", NumStr(state.ReadByte(addr + 1)));
			break;
		case 0b001: // zero page
			snprintf(addressModeDesc, sizeof(addressModeDesc), "zero page %s", NumStr(state.ReadByte(addr + 1)));
		break;
		case 0b010: // #immediate
			snprintf(addressModeDesc, sizeof(addressModeDesc), "%s", NumStr(state.ReadByte(addr + 1)));
		break;
		case 0b011: // absolute
			snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of %s", NumStr(state.ReadWord(addr + 1)));
		break;

		case 0b100: // (zero page), Y
			snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of (zero page %s) + Y", NumStr(state.ReadByte(addr + 1)));
			break;
		case 0b101: // zero page, X
			snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of zero page %s + X", NumStr(state.ReadByte(addr + 1)));
			break;
		case 0b110: // absolute, Y
			snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of %s + Y", NumStr(state.ReadWord(addr + 1)));
			break;
		case 0b111: // absolute, X
			snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of %s + X", NumStr(state.ReadWord(addr + 1)));
			break;
	}

	switch (instruction)
	{
	case 0b000:	//	ORA
		ImGui::Text("OR A with %s", addressModeDesc);
		ImGui::Text("");
		break;
	case 0b001:	//	AND
		ImGui::Text("AND A with %s", addressModeDesc);
		ImGui::Text("A AND M -> A");
		break;
	case 0b010:	//	EOR
		ImGui::Text("EOR A with %s", addressModeDesc);
		ImGui::Text("A EOR M -> A");
		break;
	case 0b011:	//	ADC
		ImGui::Text("Add A with carry to %s", addressModeDesc);
		ImGui::Text("A + M + C -> A, C");
		break;
	case 0b100:	//	STA
		ImGui::Text("Store A to %s", addressModeDesc);
		ImGui::Text("");
		break;
	case 0b101:	//	LDA
		ImGui::Text("Load A with %s", addressModeDesc);
		ImGui::Text("M -> A");
		break;
	case 0b110:	//	CMP
		ImGui::Text("Compare A to %s", addressModeDesc);
		ImGui::Text("A - M");
		break;
	case 0b111:	//	SBC
		ImGui::Text("Subtract A with carry to %s", addressModeDesc);
		ImGui::Text("");
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
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of zero page %s",NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b010://	accumulator
		snprintf(addressModeDesc,sizeof(addressModeDesc), "A");
		break;
	case 0b011://	absolute
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of %s", NumStr(state.ReadWord(addr + 1)));
		break;
	case 0b101://	zero page, X
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of zero page %s + x", NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b111://	absolute, X
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of %s + x", NumStr(state.ReadWord(addr + 1)));
		break;
	}

	switch (instruction)
	{
		case 0b000://	ASL
			ImGui::Text("Arithmetic shift left %s", addressModeDesc);
			ImGui::Text("C <- [76543210] <- 0");
			break;
		case 0b001://	ROL
			ImGui::Text("Bit rotate left %s", addressModeDesc);
			ImGui::Text("");
			break;
		case 0b010://	LSR
			ImGui::Text("Logical shift right %s", addressModeDesc);
			ImGui::Text("");
			break;
		case 0b011://	ROR
			ImGui::Text("Bit rotate right %s", addressModeDesc);
			ImGui::Text("");
			break;
		case 0b100://	STX
			ImGui::Text("Store X to %s", addressModeDesc);
			ImGui::Text("X -> M");
			break;
		case 0b101://	LDX
			ImGui::Text("Load X with %s", addressModeDesc);
			ImGui::Text("M -> X");
			break;
		case 0b110://	DEC
			ImGui::Text("Decrement memory at %s", addressModeDesc);
			ImGui::Text("M - 1 -> M");
			break;
		case 0b111://	INC
			ImGui::Text("Increment memory at %s", addressModeDesc);
			ImGui::Text("M + 1 -> M");
			break;
	}

	
}

void OutputTooltipGroupThree(FCodeAnalysisState& state, uint16_t addr, const uint8_t opcode)
{
	const uint8_t addressMode = (opcode >> 2) & 7;
	const uint8_t instruction = (opcode >> 5) & 7;
	char addressModeDesc[64];

	//The addressing modes are the same as the 10 case, except that accumulator mode is missing.
	switch (addressMode)
	{
	case 0b000://	#immediate
		snprintf(addressModeDesc, sizeof(addressModeDesc), "%s", NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b001://	zero page
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of zero page %s", NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b010://	accumulator
		snprintf(addressModeDesc, sizeof(addressModeDesc), "A");
		break;
	case 0b011://	absolute
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of %s", NumStr(state.ReadWord(addr + 1)));
		break;
	case 0b101://	zero page, X
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of zero page %s + x", NumStr(state.ReadByte(addr + 1)));
		break;
	case 0b111://	absolute, X
		snprintf(addressModeDesc, sizeof(addressModeDesc), "contents of %s + x", NumStr(state.ReadWord(addr + 1)));
		break;
	}
	switch (instruction)
	{
		case 0b001://	BIT
			ImGui::Text("Test bits with %s", addressModeDesc);
			ImGui::Text("A AND M, M7 -> N, M6 -> V");
			break;
		case 0b010://	JMP
			ImGui::Text("Jump to %s", addressModeDesc);
			ImGui::Text("(PC+1) -> PCL,	(PC + 2)->PCH");
			break;
		case 0b011://	JMP(abs)
			ImGui::Text("Jump to (%s)", NumStr(state.ReadWord(addr + 1)));
			ImGui::Text("");
			break;
		case 0b100://	STY
			ImGui::Text("Store Y in %s", addressModeDesc);
			ImGui::Text("");
			break;
		case 0b101://	LDY
			ImGui::Text("Load Y with %s", addressModeDesc);
			ImGui::Text("");
			break;
		case 0b110://	CPY
			ImGui::Text("Compare Y to %s", addressModeDesc);
			ImGui::Text("Y - M");
			break;
		case 0b111://	CPX
			ImGui::Text("Compare X to %s", addressModeDesc);
			ImGui::Text("X - M");
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