#include "M6502Disassembler.h"

uint16_t M6502DisassembleCodeInfoItem(uint16_t pc, FCodeAnalysisState& state, FCodeInfo* pCodeInfo)
{
	// TODO: Implement
	return pc;
}

uint16_t M6502DisassembleGetNextPC(uint16_t pc, FCodeAnalysisState& state, uint8_t& opcode)
{
	// TODO: Implement
	return pc;
}

std::string M6502GenerateDasmStringForAddress(FCodeAnalysisState& state, uint16_t pc, ENumberDisplayMode hexMode)
{
	// TODO: Implement
	return std::string("Not Implemented");
}
