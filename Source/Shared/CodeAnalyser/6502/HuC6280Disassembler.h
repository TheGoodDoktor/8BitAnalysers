#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <Util/Misc.h>

class FCodeAnalysisState;
class FExportDasmState;
struct FCodeInfo;

struct FHuC6280DisassemblerConfig
{
	char BrOp;					// Bracket open character
	char BrCl;					// Bracket close character
	const char* ZpPr;			// Zero Page prefix. Only set when doing asm export.
	bool MprIndexMode;		// Convert mpr bitfield to mpr index for TMA & TAM instructions.
};

uint16_t HuC6280DisassembleCodeInfoItem(uint16_t pc, FCodeAnalysisState& state, FCodeInfo* pCodeInfo);
uint16_t HuC6280DisassembleGetNextPC(uint16_t pc, FCodeAnalysisState& state, std::vector<uint8_t>& opcodes);
bool HuC6280GenerateDasmExportString(FExportDasmState& exportState);

FHuC6280DisassemblerConfig& GetHuC6280DisassemblerConfig();
const FHuC6280DisassemblerConfig& GetHuC6280DisassemblerDefaultConfig();