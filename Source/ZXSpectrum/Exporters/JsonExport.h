#pragma once

class FCodeAnalysisState;
class FSpectrumEmu;

bool ExportROMJson(FCodeAnalysisState& state, const char* pJsonFileName);
bool ExportGameJson(FSpectrumEmu* pSpectrumEmu, const char* pJsonFileName);
bool ImportAnalysisJson(FSpectrumEmu* pSpectrumEmu, const char* pJsonFileName);
