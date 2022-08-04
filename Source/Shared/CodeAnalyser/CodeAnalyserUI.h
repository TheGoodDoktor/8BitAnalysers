#pragma once
#include <cstdint>

struct FSpeccyUI;
struct FCodeAnalysisState;
struct FDataInfo;

// UI
void DrawCodeAddress(FCodeAnalysisState &state, uint16_t addr);
void DrawAddressLabel(FCodeAnalysisState &state, uint16_t addr);

void DrawCodeAnalysisData(FCodeAnalysisState &state);
void DrawGlobals(FCodeAnalysisState &state);

void DrawDataDetails(FCodeAnalysisState &state, FDataInfo *pDataInfo);

void CodeAnalyserGoToAddress(uint16_t newAddress, bool bLabel = false);