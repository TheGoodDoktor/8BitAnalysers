#pragma once

struct FCodeAnalysisState;
class FSpectrumEmu;

bool ExportROMJson(FCodeAnalysisState& state, const char* pJsonFileName);
bool ExportGameJson(FSpectrumEmu* pSpectrumEmu, const char* pJsonFileName);
bool ImportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName);