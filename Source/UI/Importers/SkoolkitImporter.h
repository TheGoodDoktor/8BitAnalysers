#pragma once

struct FCodeAnalysisState;
struct FSkoolFileInfo;

bool ImportSkoolKitFile(FCodeAnalysisState& state, const char* pTextFileName, FSkoolFileInfo* pSkoolInfo =nullptr);
