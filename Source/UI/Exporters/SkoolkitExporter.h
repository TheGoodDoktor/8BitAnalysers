#pragma once
#include <stdint.h>

struct FCodeAnalysisState;

bool ExportSkoolKitControlFile(FCodeAnalysisState& state, const char* pTextFileName, const char* pGameName, uint16_t startAddr, uint16_t endAddr);
