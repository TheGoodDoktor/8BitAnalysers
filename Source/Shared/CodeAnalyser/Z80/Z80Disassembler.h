#pragma once

#include <stdint.h>
#include <string>
#include <Util/Misc.h>

class FCodeAnalysisState;
struct FCodeInfo;

uint16_t Z80DisassembleCodeInfoItem(uint16_t pc, FCodeAnalysisState& state, FCodeInfo* pCodeInfo);
uint16_t Z80DisassembleGetNextPC(uint16_t pc, FCodeAnalysisState& state, uint8_t& opcode);
std::string Z80GenerateDasmStringForAddress(FCodeAnalysisState& state, uint16_t pc, ENumberDisplayMode hexMode);
