#pragma once

class FCodeAnalysisState;
struct FSkoolFileInfo;

bool ImportSkoolKitFile(FCodeAnalysisState& state, const char* pTextFileName, FSkoolFileInfo* pSkoolInfo =nullptr);
