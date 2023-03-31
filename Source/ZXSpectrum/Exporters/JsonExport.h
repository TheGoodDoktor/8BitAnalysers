#pragma once

class FCodeAnalysisState;
class FSpectrumEmu;

bool ExportROMAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName);
bool ExportGameAnalysisJson(FSpectrumEmu* pSpectrumEmu, const char* pJsonFileName);
bool ImportAnalysisJson(FSpectrumEmu* pSpectrumEmu, const char* pJsonFileName);

bool ExportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile);
bool ImportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile);
