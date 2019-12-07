#pragma once
#include <cstdint>
#include <string>
#include "SpeccyUI.h"

enum class LabelType;
struct FSpeccyUI;

// Analysis
void InitialiseCodeAnalysis(FSpeccyUI *pUI);
bool GenerateLabelForAddress(FSpeccyUI *pUI, uint16_t pc, LabelType label);
void RunStaticCodeAnalysis(FSpeccyUI *pUI, uint16_t pc);

// UI
void DrawCodeAnalysisData(FSpeccyUI *pUI);