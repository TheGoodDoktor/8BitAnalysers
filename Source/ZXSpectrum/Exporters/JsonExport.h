#pragma once

struct FCodeAnalysisState;

bool ExportROMJson(FCodeAnalysisState& state, const char* pJsonFileName);
bool ExportGameJson(FCodeAnalysisState& state, const char* pTextFileName);
bool ImportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName);