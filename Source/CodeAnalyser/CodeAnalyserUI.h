#pragma once
#include <cstdint>

struct FSpeccyUI;
struct FCodeAnalysisState;

// UI
void DrawCodeAddress(FCodeAnalysisState &state, uint16_t addr);
void DrawAddressLabel(FCodeAnalysisState &state, uint16_t addr);

void DrawCodeAnalysisData(FCodeAnalysisState &state);
void DrawGlobals(FCodeAnalysisState &state);