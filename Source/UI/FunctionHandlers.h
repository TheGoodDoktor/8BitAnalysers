#pragma once
#include <cstdint>

struct FSpeccyUI;

int FunctionTrapFunction(uint16_t pc, uint16_t nextpc, int ticks, uint64_t pins, FSpeccyUI *pUI);

void DrawFunctionInfo(FSpeccyUI* pUI);