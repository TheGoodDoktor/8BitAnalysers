#pragma once

class FCodeAnalysisState;
struct FCodeInfo;

#include <map>
#include <cinttypes>

struct FInstructionInfo
{
	const char* Description = nullptr;
};

typedef std::map<uint8_t, FInstructionInfo> InstructionInfoMap;

void ShowCodeToolTip(FCodeAnalysisState& state, uint16_t addr);