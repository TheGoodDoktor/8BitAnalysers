#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <Util/Misc.h>

class FCodeAnalysisState;
class FExportDasmState;
struct FCodeInfo;

uint16_t HuC6280DisassembleCodeInfoItem(uint16_t pc, FCodeAnalysisState& state, FCodeInfo* pCodeInfo);
uint16_t HuC6280DisassembleGetNextPC(uint16_t pc, FCodeAnalysisState& state, std::vector<uint8_t>& opcodes);
bool HuC6280GenerateDasmExportString(FExportDasmState& exportState);

