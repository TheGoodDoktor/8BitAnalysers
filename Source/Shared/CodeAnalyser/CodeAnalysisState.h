#pragma once

class FCodeAnalysisState;

bool ExportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile, bool bExportROMInfo = false);
bool ImportAnalysisState(FCodeAnalysisState& state, const char* pAnalysisBinFile);
