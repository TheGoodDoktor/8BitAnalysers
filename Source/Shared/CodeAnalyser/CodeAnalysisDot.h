#pragma once

class FCodeAnalysisState;

bool ExportCodeAnalysisDot(const FCodeAnalysisState& state, const char* pFilename, bool bROMS = false);