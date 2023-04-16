#pragma once

class FCodeAnalysisState;
class FSpectrumEmu;

//bool SaveGameData(FSpectrumEmu* pSpectrumEmu, const char* fname);
bool LoadGameData(FSpectrumEmu* pSpectrumEmu, const char* fname);

bool SaveROMData(const FCodeAnalysisState& state, const char* fname);
bool LoadROMData(FCodeAnalysisState& state, const char* fname);

bool SaveGameState(FSpectrumEmu* pSpectrumEmu, const char* fname);
bool LoadGameState(FSpectrumEmu* pSpectrumEmu, const char* fname);
