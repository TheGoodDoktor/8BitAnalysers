#pragma once

class FCodeAnalysisState;

bool ExportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName);
bool ImportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName);
