#pragma once

class FCodeAnalysisState;

bool ExportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName, bool bROMS = false);
bool ImportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName);
