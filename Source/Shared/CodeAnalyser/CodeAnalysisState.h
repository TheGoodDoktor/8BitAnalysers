#pragma once

class FCodeAnalysisState;

bool ExportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile);
bool ImportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile);
