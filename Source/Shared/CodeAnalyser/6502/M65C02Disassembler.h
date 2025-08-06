#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <Util/Misc.h>

class FCodeAnalysisState;
class FExportDasmState;
struct FCodeInfo;

uint16_t M65C02DisassembleCodeInfoItem(uint16_t pc, FCodeAnalysisState& state, FCodeInfo* pCodeInfo);
uint16_t M65C02DisassembleGetNextPC(uint16_t pc, FCodeAnalysisState& state, std::vector<uint8_t>& opcodes);
bool M65C02GenerateDasmExportString(FExportDasmState& exportState);

