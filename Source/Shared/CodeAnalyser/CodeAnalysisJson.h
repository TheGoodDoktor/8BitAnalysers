#pragma once

class FCodeAnalysisState;

enum class EExportAnalysis
{
	ROM,
	RAM,
	Both
};

bool ExportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName, bool bROMS = false);
bool ExportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName, EExportAnalysis exportType);
bool ImportAnalysisJson(FCodeAnalysisState& state, const char* pJsonFileName);
