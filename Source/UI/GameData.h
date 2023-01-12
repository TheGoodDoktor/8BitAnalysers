#pragma once

struct FCodeAnalysisState;

bool SaveGameData(const FCodeAnalysisState& state, const char* fname);
bool LoadGameData(FCodeAnalysisState& state, const char* fname);
bool SaveROMData(const FCodeAnalysisState& state, const char* fname);
bool LoadROMData(FCodeAnalysisState& state, const char* fname);
