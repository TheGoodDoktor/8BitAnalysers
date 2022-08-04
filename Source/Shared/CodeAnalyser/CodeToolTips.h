#pragma once

struct FCodeAnalysisState;
struct FCodeInfo;

#include <map>

typedef std::map<uint8_t, const char*> InstructionInfoMap;

void ShowCodeToolTip(FCodeAnalysisState &state, const FCodeInfo *pCodeInfo);
